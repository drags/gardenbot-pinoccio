#!/usr/bin/env python

import json
import socket
import requests
import argparse
import StringIO


class StripAction(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        values = values.rstrip('.')
        setattr(namespace, self.dest, values)

ap = argparse.ArgumentParser(description='Pipe Pinocc.io api to graphite')
ap.add_argument('-A', '--auth', help='Token for accessing api.pinocc.io',
                required=True, metavar='API_TOKEN')
ap.add_argument('-a', '--account_id', help='Account ID (numeric)', required=True)
ap.add_argument('-t', '--troop_id', help='Troop ID (numeric)', required=True)
ap.add_argument('-s', '--scout_id', help='Scout ID (numeric)', required=True)
ap.add_argument('-C', '--carbon_server', help='Carbon server address',
                required=True)
ap.add_argument('-P', '--carbon_port', help='Carbon server port', default=2003)
ap.add_argument('-p', '--prefix', help='Metric prefix (default: gardenbot.',
                default='gardenbot', action=StripAction)
args = ap.parse_args()


def graphite_send(msg):
    sock = socket.socket()
    sock.connect((args.carbon_server, args.carbon_port))
    sock.sendall(msg)
    sock.close()


def send_data_to_graphite(msg):
    msg_time = int(msg.pop('_t'))/1000
    msg_type = msg.pop('type')
    graphite_msg = StringIO.StringIO()

    for k in msg:
        graphite_msg.write('%s.%s.%s %s %s\n' % (args.prefix, msg_type, k, msg[k], msg_time)) #noqa

    _msg = graphite_msg.getvalue()

    print _msg
    graphite_send(_msg)
    graphite_msg.close()

try:
    r = requests.get('https://api.pinocc.io/v1/sync', data={'token': args.auth},
                                                            stream=True)
except requests.exceptions.ConnectionError as e:
    print "Unable to connect to pinnocio API:", e.msg

for msg in r.iter_lines():
    try:
        _d = json.loads(msg)
    except ValueError:
        print "Got non json message: ", msg
        continue

    msg_data = _d['data']
    msg_content = msg_data['value']

    # Filter for correct scout
    if not set(('account', 'troop', 'scout')).issubset(msg_data.keys()):
        continue

    account_match = msg_data['account'] == args.account_id
    troop_match = msg_data['troop'] == args.troop_id
    scout_match = msg_data['scout'] == args.scout_id
    value_dict = type(msg_data['value']) == dict

    if account_match and troop_match and scout_match and value_dict:
        send_data_to_graphite(msg_content)
