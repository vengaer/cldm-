from cgen import *
from cldm import *
from config import *
from makegen import *
from runner import *
from util import *

__TESTFILE = 'test.c'
def gen_makefile(src):
    mgen = Makegen('runtest', src=src)
    mgen.adjust('CFLAGS', '-fPIC', Mod.REMOVE)
    mgen.adjust('LDFLAGS', '-shared', Mod.REMOVE)
    mgen.adjust('LDFLAGS', '-L{}'.format(project_root), Mod.APPEND)
    mgen.adjust('LDLIBS', '-lcldm -lcldm_main')
    mgen.export('LD_LIBRARY_PATH', project_root)
    mgen.generate()

def test_triggered_assertion_causes_failure():
    cgen = CGen(__TESTFILE)
    cgen.append_include('cldm.h', system_header=False)

    with cgen.open_macro('TEST', 'foo'):
        cgen.append_line('ASSERT_EQ(1,2);')
    with cgen.open_macro('TEST', 'bar'):
        pass
    with cgen.open_macro('TEST', 'baz'):
        pass

    cgen.write()
    gen_makefile(__TESTFILE)

    run(ContainsMatcher(r'foo.*\(\d+/\d+\).*fail'), ContainsMatcher(r'1/1\s*assertions\s*failed\s*across\s*1\s*test'), RvDiffMatcher(0))

def test_violating_assertions_logged():
    cgen = CGen(__TESTFILE)
    cgen.append_include('cldm.h', system_header=False)

    errout = [ \
        '1.*==.*2',
        '3.*<=.*1',
        '3.*<.*1',
        '1.*>=.*3',
        '1.*>.*3',
        '1.*!=.*1',
        '!.*1',
        '0'
    ]

    with cgen.open_macro('TEST', 'foo'):
        cgen.append_line('ASSERT_EQ(1,2);')     \
            .append_line('ASSERT_LE(3,1);')     \
            .append_line('ASSERT_LT(3,1);')     \
            .append_line('ASSERT_GE(1,3);')     \
            .append_line('ASSERT_GT(1,3);')     \
            .append_line('ASSERT_NE(1,1);')     \
            .append_line('ASSERT_FALSE(1);')    \
            .append_line('ASSERT_TRUE(0);')
    with cgen.open_macro('TEST', 'bar'):
        pass
    with cgen.open_macro('TEST', 'baz'):
        pass

    cgen.write()
    gen_makefile(__TESTFILE)

    run(ContainsMatcher(r'foo.*\(\d+/\d+\).*fail'), ContainsMatcher('8/8.*1.*test.*{}'.format('.*'.join(errout))), RvDiffMatcher(0))

def test_number_of_failed_functions_logged():
    cgen = CGen(__TESTFILE)
    cgen.append_include('cldm.h', system_header=False)

    with cgen.open_macro('TEST', 'foo'):
        cgen.append_line('ASSERT_EQ(1,2);')
    with cgen.open_macro('TEST', 'bar'):
        cgen.append_line('ASSERT_EQ(1,2);')
    with cgen.open_macro('TEST', 'baz'):
        cgen.append_line('ASSERT_EQ(1,2);')

    cgen.write()
    gen_makefile(__TESTFILE)

    run(ContainsMatcher(r'(foo|bar|baz).*fail.*(foo|bar|baz).*fail.*(foo|bar|baz).*fail'),
        ContainsMatcher(r'3/3\s*assertions\s*failed\s*across\s*3\s*tests'),
        RvDiffMatcher(0))

def test_fails_only_on_assertion_triggered():
    cgen = CGen(__TESTFILE)
    cgen.append_include('cldm.h', system_header=False)

    with cgen.open_macro('TEST', 'foo'):
        cgen.append_line('ASSERT_EQ(1,1);')
    with cgen.open_macro('TEST', 'bar'):
        cgen.append_line('ASSERT_GT(8,2);')
    with cgen.open_macro('TEST', 'baz'):
        cgen.append_line('ASSERT_LE(1,2);')

    cgen.write()
    gen_makefile(__TESTFILE)

    run(ContainsMatcher(r'(foo|bar|baz).*pass.*(foo|bar|baz).*pass.*(foo|bar|baz).*pass'), rvmatcher = RvEqMatcher(0))
