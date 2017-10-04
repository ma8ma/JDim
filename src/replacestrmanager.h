// ライセンス: GPL2

//
// 文字列置換の管理クラス
//

#ifndef _REPLACESTRMANAGER_H
#define _REPLACESTRMANAGER_H

#include "jdlib/jdregex.h"

#include <list>
#include <memory>
#include <string>

namespace XML
{
    class Dom;
}

namespace CORE
{
    enum
    {
        REPLACETARGET_SUBJECT = 0,
        REPLACETARGET_NAME,
        REPLACETARGET_MAIL,
        REPLACETARGET_DATE,
        REPLACETARGET_MESSAGE,
        REPLACETARGET_MAX
    };

    union ReplaceStrCondition
    {
        struct {
            unsigned int active : 1;
            unsigned int icase  : 1;
            unsigned int regex  : 1;
            unsigned int wchar  : 1;
            unsigned int norm   : 1;
            unsigned int : 27;
        } flag;
        int raw;
    };

    struct ReplaceStrItem
    {
        int condition;
        std::string pattern;
        std::string replace;
        JDLIB::RegexPattern creg;
    };

    using ReplaceStrItemList = std::list< std::unique_ptr< ReplaceStrItem > >;

    class ReplaceStr_Manager
    {
        ReplaceStrItemList m_list[ REPLACETARGET_MAX ];
        bool m_chref[ REPLACETARGET_MAX ];

    public:

        ReplaceStr_Manager();
        ~ReplaceStr_Manager() noexcept;

        const ReplaceStrItemList& get_list( const int id ) const { return m_list[ id ]; }
        size_t list_size( const int id ) const { return m_list[ id ].size(); }
        void list_clear( const int id );
        void list_append( const int id, const int condition, const std::string& pattern, const std::string& replace );

        bool get_chref( const int id ) const { return m_chref[ id ]; }
        void set_chref( const int id, const bool chref ){ m_chref[ id ] = chref; }

        void save_xml();

        // 文字列を置換
        std::string replace( const char* str, const int lng, const int id ) const;

        static XML::Dom* dom_append( XML::Dom* node, const int id, const bool chref );
        static XML::Dom* dom_append( XML::Dom* node, const int condition, const std::string& pattern,
                                     const std::string& replace );

      private:

        void xml2list( const std::string& xml );

        static int target_id( const std::string& name );
        static std::string target_name( const int id );
    };

    ///////////////////////////////////////
    // インターフェース

    ReplaceStr_Manager* get_replacestr_manager();
    void delete_replacestr_manager();
}


#endif
