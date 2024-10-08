#include <stdio.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// 定义枚举和描述
#define ENUM_NAMES(name, desc) enum_names(name, desc, #name)
#define ENUMNAMES_SECTION \
    ENUM_NAMES(STREAMER_ROOM_IMG, "直播间标志搜索区域") \
    ENUM_NAMES(STREAMER, "主播")

// 定义枚举类型
typedef enum {
#define enum_names(enum_code, enum_desc, enum_name) enum_code,
    ENUMNAMES_SECTION
#undef enum_names
} TRZCODE;

// 生成 sections 数组的宏
#define enum_names(enum_code, enum_desc, enum_name) \
    TOSTRING(enum_code) ": " enum_desc " (" enum_name ")",

const char* sections[] = {
    ENUMNAMES_SECTION
};

int main() {
    for (int i = 0; i < sizeof(sections) / sizeof(sections[0]); i++) {
        printf("%s\n", sections[i]);
    }
    return 0;
}
