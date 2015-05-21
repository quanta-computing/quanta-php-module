import argparse

from dockerunner.builder import Builder
from dockerunner.runner import Runner
from dockerunner.config import load_config
from dockerunner.spec import Suite


VALID_ACTIONS = ['build', 'run']

def _title(title, char='#', width=80):
    """
    Returns a pretty title

    """
    out = '#' * width + "\n"
    out += '##' + ' ' * (width // 2 - 2 - (len(title) // 2 + len(title) % 2))
    out += title.upper()
    out += ' ' * (width // 2 + width % 2 - 2 - len(title) // 2) + '##'
    out += "\n" + '#' * width + "\n"
    return out


def main():
    parser = argparse.ArgumentParser(
        description='This program is an awesome test runner around Docker containers',
        epilog='Copyright Quanta-computing 2014',
    )
    parser.add_argument('-d', '--debug', action='store_true', help='debug mode')
    parser.add_argument('--no-build-docker', action='store_true', help='disable build of docker images')
    parser.add_argument('--no-build-sql', action='store_true', help='disable sqldump imports')
    parser.add_argument('--test', action='append', help='test suites to run (default all)')
    parser.add_argument('actions', nargs='*', choices=VALID_ACTIONS + [[]],
        help='what to do (default all)',
    )
    opts = parser.parse_args()
    if not opts.actions:
        opts.actions = VALID_ACTIONS
    if opts.test is not None and len(opts.test) == 0:
        opts.test = None
    config = load_config()
    builder = Builder(config, debug=opts.debug,
        without_docker=opts.no_build_docker, without_sqldump=opts.no_build_sql)
    runner = Runner(config)
    test_suites = Suite(config['global']['spec_root'], opts.test)
    builder.prepare()
    if 'build' in opts.actions:
        print(_title("build phase"))
        builder.build()
        print("\n")
    if 'run' in opts.actions:
        print(_title("run phase"))
        for build in builder.builds:
            runner.run(build, test_suites)
            print("\n{}\n".format('-' * 80))


if __name__ == "__main__":
    main()
