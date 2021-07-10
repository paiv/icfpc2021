#!/usr/bin/env python
import configparser
import http.client
import json as json_parser
import logging
import time
from pathlib import Path
from urllib.parse import urljoin, urlparse


logging.basicConfig(level=logging.DEBUG)


class PortalClient:
    def __init__(self, endpoint=None, api_key=None):
        self.logger = logging.getLogger(self.__class__.__name__)
        self._read_env()
        if endpoint:
            self.endpoint = endpoint
        if api_key:
            self.api_key = api_key
        self.conn = None

    def _read_env(self):
        env = Path(__file__).with_name('.env')
        if env.is_file():
            config = configparser.ConfigParser()
            config['default'] = {'ENDPOINT': 'https://poses.live'}
            config.read(env)
            self.endpoint = config['default']['ENDPOINT']
            self.api_key = config['default']['API_KEY']

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def _open_conn(self):
        if not self.conn:
            o = urlparse(self.endpoint)
            self.logger.debug(f'connect https://{o.netloc}')
            self.conn = http.client.HTTPSConnection(o.hostname, port=o.port)

    def close(self):
        if self.conn:
            o = urlparse(self.endpoint)
            self.conn.close()
            self.conn = None

    def _request(self, method, url, json=None, headers=None):
        body = json_parser.dumps(json, separators=',:') if json else None
        o = urlparse(url)
        self._open_conn()
        self.logger.debug(f'{method} {o.path}')
        self.conn.request(method, o.path, body=body, headers=headers)
        r = self.conn.getresponse()
        if not (200 <= r.status < 300):
            raise Exception((r.status, r.reason))
        chunked = r.headers['Content-Length'] is None
        content = r.read()
        if not content and chunked:
            time.sleep(0.5)
            content = r.read()
        return json_parser.loads(content)

    def get(self, path):
        url = urljoin(self.endpoint, path)
        headers = {'Authorization': f'Bearer {self.api_key}'}
        return self._request('GET', url, headers=headers)

    def post(self, path, obj):
        url = urljoin(self.endpoint, path)
        headers = {'Authorization': f'Bearer {self.api_key}'}
        return self._request('POST', url, json=obj, headers=headers)

    def hello(self):
        return self.get('/api/hello')

    def get_problem(self, n):
        return self.get(f'/api/problems/{n}')

    def post_solution(self, n, sol):
        return self.post(f'/api/problems/{n}/solutions', sol)

    def get_info(self, n, sid):
        return self.get(f'/api/problems/{n}/solutions/{sid}')


def gethello(args):
    with PortalClient() as cli:
        print(cli.hello())


def grab_problems(args):
    start = args.start
    stop = args.stop or start + 1
    path = Path(__file__).with_name('spec') / 'problems'
    with PortalClient() as cli:
        for i in range(start, stop):
            o = cli.get_problem(i)
            with open(path / f'{i}.json', 'w') as fp:
                json_parser.dump(o, fp, separators=',:')
            time.sleep(0.25)


def submit_solution(args):
    n = args.solution
    path = Path(__file__).with_name('solutions') / f'{n}.json'
    with open(path) as fp:
        sol = json_parser.load(fp)

    with PortalClient() as cli:
        cli.post_solution(n, sol)


def getinfo(args):
    n = args.problem
    sid = args.pose
    with PortalClient() as cli:
        print(cli.get_info(n, sid))


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Portal client')
    def print_help(args):
        parser.print_help()
    parser.set_defaults(func=print_help)
    subp = parser.add_subparsers()

    p = subp.add_parser('hello', help='Fetch hello')
    p.set_defaults(func=gethello)

    p = subp.add_parser('grab', help='Fetch problems')
    p.add_argument('start', metavar='N', type=int, help='Problem number (range start)')
    p.add_argument('stop', metavar='M', nargs='?', type=int, help='Problem number (range stop)')
    p.set_defaults(func=grab_problems)

    p = subp.add_parser('submit', help='Submit solution')
    p.add_argument('solution', metavar='N', type=int, help='Solution number')
    p.set_defaults(func=submit_solution)

    p = subp.add_parser('info', help='Get information')
    p.add_argument('problem', metavar='N', type=int, help='Problem number')
    p.add_argument('pose', metavar='S', help='Pose submission id')
    p.set_defaults(func=getinfo)

    args = parser.parse_args()
    args.func(args)
