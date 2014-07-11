#!/usr/bin/env python

import json
import socket
import requests
import argparse
import StringIO
import time


class StripAction(argparse.Action):
    '''Strip trailing period from argparse argument'''
    def __call__(self, parser, namespace, values, option_string=None):
        values = values.rstrip('.')
        setattr(namespace, self.dest, values)

ap = argparse.ArgumentParser(description='Pipe Pinocc.io api to graphite')
ap.add_argument('-A', '--auth', help='Token for accessing api.pinocc.io',
                required=True, metavar='API_TOKEN')
ap.add_argument('-a', '--account_id', help='Filter by account ID (numeric)')
ap.add_argument('-t', '--troop_id', help='Filter by troop ID (numeric)')
ap.add_argument('-s', '--scout_id', help='Filter by scout ID (numeric)')
ap.add_argument('-C', '--carbon_server', help='Carbon server address',
                required=True)
ap.add_argument('-P', '--carbon_port', help='Carbon server port', default=2003)
ap.add_argument('-p', '--prefix', help='Metric prefix (default: gardenbot.',
                default='gardenbot', action=StripAction)
args = ap.parse_args()


def graphite_send(msg):
    '''Bare socket push to graphite'''
    sock = socket.socket()
    sock.connect((args.carbon_server, args.carbon_port))
    sock.sendall(msg)
    sock.close()


def send_data_to_graphite(msg):
    '''Parse Pinoccio message and hand to graphite_send'''
    msg_time = int(msg.pop('_t'))/1000  # time in milliseconds
    msg_type = msg.pop('type')
    graphite_msg = StringIO.StringIO()  # buffer for graphite payload

    for k in msg:
        # Filter keys with non numeric values
        try:
            float(msg[k])
        except (ValueError, TypeError):
            continue

        graphite_msg.write('%s.%s.%s %s %s\n' % (args.prefix, msg_type, k, msg[k], msg_time)) #noqa

    _msg = graphite_msg.getvalue()
    # print _msg # debug
    graphite_send(_msg)
    graphite_msg.close()


while True:
    # Connect to streaming API
    try:
        r = requests.get('https://api.pinocc.io/v1/sync', data={'token': args.auth}, stream=True) #noqa
    except requests.exceptions.ConnectionError as e:
        print "Unable to connect to pinnocio API:", e
        time.sleep(30)
        continue

    if not r.ok:
        print "Failed to fetch data: ", r.text
        time.sleep(30)
        continue

    # Process streaming response
    for msg in r.iter_lines():
        try:
            msg_json = json.loads(msg)
        except ValueError:
            print "Got non json message: ", msg
            continue

        # {'data':{...'value':{'type':"",'_t':"",'custom_1..n'}}}
        msg_data = msg_json['data']
        msg_content = msg_data['value']

        if not set(('account', 'troop', 'scout')).issubset(msg_data.keys()):
            print "Got non scout-identifying message: ", msg
            continue

        # Filter for correct scout, when filters are set
        account_match = (msg_data['account'] == args.account_id) if args.account_id is not None else True #noqa
        troop_match = (msg_data['troop'] == args.troop_id) if args.troop_id is not None else True #noqa
        scout_match = (msg_data['scout'] == args.scout_id) if args.scout_id is not None else True #noqa
        value_dict = type(msg_data['value']) == dict  # TODO !str?

        if account_match and troop_match and scout_match and value_dict:
            send_data_to_graphite(msg_content)
