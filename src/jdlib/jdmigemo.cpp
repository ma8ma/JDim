// ライセンス: GPL2

//
// Thanks to 「テスト運用中」スレの18氏
//
// http://jd4linux.sourceforge.jp/cgi-bin/bbs/test/read.cgi/support/1149945056/18
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_MIGEMO_H

//#define _DEBUG
#include "jddebug.h"

#include "jdmigemo.h"

#include <string>
#include <vector>

migemo *migemo_object;


std::string jd_migemo_regcreate(const char* regex)
{
    migemo* m = migemo_object;
    if(!m){
        return {};
    }
    //migemo_load(m,MIGEMO_DICTID_MIGEMO,JD_MIGEMO_DICTNAME);
    if(!migemo_is_enable(m)){
        return {};
    }

    std::vector<unsigned char> utmp;
    while(*regex != '\0' && *regex != '\n') {
        utmp.push_back(*regex);
        ++regex;
    }
    utmp.push_back('\0');

    unsigned char *owner = migemo_query(m, utmp.data());
    std::string ctmp{ reinterpret_cast<char*>(owner) };

#ifdef _DEBUG
    std::cout << "migemo comp:" << ctmp << std::endl;
#endif

    migemo_release(m, owner);
    return ctmp;
}


int jd_migemo_init(const char* filename)
{
#ifdef _DEBUG
	std::cout << "migemo-dict: " << filename << std::endl;
#endif
    migemo_object = migemo_open(filename);
    if(migemo_is_enable(migemo_object)){
        return 1;
    }else{
        migemo_close(migemo_object);
        migemo_object = nullptr;
        return 0;
    }
}


int jd_migemo_close()
{
    migemo_close(migemo_object);
    migemo_object = nullptr;
    return 1;
}

#endif
