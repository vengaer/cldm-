#include <cldm/cldm.h>
#include <cldm/cldm_log.h>
#include <cldm/cldm_ntbs.h>

#include <string.h>

#include <sys/types.h>

TEST(cldm_ntbschr) {
    char const *str = "a string with spaces";
    ASSERT_EQ(cldm_ntbschr((char *)str, ' '), str + 1);
    str = "some/path/to/a/file";
    ASSERT_EQ(cldm_ntbschr((char *)str, '/'), str + 4);
    str = "a string without slashes";
    ASSERT_EQ(cldm_ntbschr((char *)str, '/'), 0);
}

TEST(cldm_ntbscrchr) {
    char const *str = "a string with spaces";
    ASSERT_EQ(cldm_ntbscrchr(str, ' '), str + 13);
    str = "/some/path/to/a/file";
    ASSERT_EQ(cldm_ntbscrchr(str, '/'), str + 15);
    str = "a_string";
    ASSERT_EQ(cldm_ntbscrchr(str, ' '), 0);
}

TEST(cldm_ntbslen) {
    ASSERT_EQ(cldm_ntbslen("a string"), 8);
    ASSERT_EQ(cldm_ntbslen("another string"), 14);
    ASSERT_EQ(cldm_ntbslen("a"), 1);
}

TEST(cldm_ntbscpy) {
    enum { BUFSIZE = 64 };
    char src[BUFSIZE];
    char dst[BUFSIZE];

    strcpy(src, "a string");

    ASSERT_EQ(cldm_ntbscpy(dst, src, sizeof(src)), (ssize_t)strlen(src));
    ASSERT_EQ(strcmp(dst, src), 0);

    ASSERT_LT(cldm_ntbscpy(dst, src, 4), 0);
    ASSERT_EQ(dst[3], 0);
    ASSERT_EQ(strcmp(dst, "a s"), 0);

    ASSERT_EQ(cldm_ntbscpy(dst, src, 9), (ssize_t)strlen(src));
    ASSERT_EQ(dst[strlen(src)], 0);
}
