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

        ArticleMachi( const std::string& datbase, const std::string& id, bool cached, const Encoding enc );
        ~ArticleMachi() noexcept;

        // 書き込みメッセージ変換
        std::string create_write_message( const std::string& name, const std::string& mail,
                                          const std::string& msg ) override;

        // bbscgi のURL
        std::string url_bbscgi() const override;

        // subbbscgi のURL
        std::string url_subbbscgi() const override;

      private:
        
        // offlawモードなら更新チェック可能
        bool enable_check_update() const override;

        NodeTreeBase* create_nodetree() override;
    };
}

#endif
