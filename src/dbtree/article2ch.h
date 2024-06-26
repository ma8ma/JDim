// ライセンス: GPL2

//
// 2ch型スレ情報クラス
//

#ifndef _ARTICLE2ch_H
#define _ARTICLE2ch_H

#include "article2chcompati.h"

namespace DBTREE
{
    class Article2ch : public Article2chCompati
    {
      public:

        Article2ch( const std::string& datbase, const std::string& id, bool cached, const Encoding enc );
        ~Article2ch() noexcept;

        // 書き込みメッセージ変換
        std::string create_write_message( const std::string& name, const std::string& mail,
                                          const std::string& msg ) override;

      protected:

        // dat落ちしたスレをロードするか
        bool is_load_olddat() const override;

      private:
        
        NodeTreeBase* create_nodetree() override;
    };
}

#endif
