// ライセンス: GPL2

//
// まち型スレ情報クラス
//

#ifndef _ARTICLEMACHI_H
#define _ARTICLEMACHI_H

#include "articlebase.h"

namespace DBTREE
{
    class NodeTreeBase;

    class ArticleMachi : public ArticleBase
    {
      public:

        ArticleMachi( const std::string& datbase, const std::string& id, bool cached );
        ~ArticleMachi();

        // 書き込みメッセージ変換
        virtual const std::string create_write_message( const std::string& name, const std::string& mail, const std::string& msg );

        // bbscgi のURL
        virtual const std::string url_bbscgi();
        
        // subbbscgi のURL
        virtual const std::string url_subbbscgi();

      private:
        
        // offlawモードなら更新チェック可能
        virtual const bool enable_check_update();

        virtual NodeTreeBase* create_nodetree();
    };
}

#endif
