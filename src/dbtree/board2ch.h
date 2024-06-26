// ライセンス: GPL2

//
// 2ch 
//

#ifndef _BOARD2CH_H
#define _BOARD2CH_H

#include "board2chcompati.h"

#include <memory>


namespace DBTREE
{
    class FrontLoader;

    enum
    {
        DEFAULT_NUMBER_MAX_2CH = 1000,  // デフォルト最大レス数
        DEFAULT_MAX_DAT_LNG = 512 // デフォルトのdatの最大サイズ(Kバイト)
    };

    class Board2ch : public Board2chCompati
    {
        std::unique_ptr<FrontLoader> m_frontloader;

      public:

        Board2ch( const std::string& root, const std::string& path_board,const std::string& name );
        ~Board2ch() noexcept;

        // ユーザーエージェント
        const std::string& get_agent() const override; // ダウンロード用
        const std::string& get_agent_w() const override; // 書き込み用

        // 読み込み用プロキシ
        std::string get_proxy_host() const override;
        int get_proxy_port() const override;
        std::string get_proxy_basicauth() const override;

        // 書き込み用プロキシ
        std::string get_proxy_host_w() const override;
        int get_proxy_port_w() const override;
        std::string get_proxy_basicauth_w() const override;

        // 読み込み用クッキー
        std::string cookie_for_request() const override;
        // 書き込み用クッキー
        std::string cookie_for_post() const override;

        // 書き込み時のリファラ
        std::string get_write_referer( const std::string& url ) override;

        // フロントページのダウンロード
        void download_front() override;

        // 新スレ作成用のメッセージ変換
        std::string create_newarticle_message( const std::string& subject, const std::string& name,
                                               const std::string& mail, const std::string& msg ) override;

        // 新スレ作成用のbbscgi のURL
        std::string url_bbscgi_new() const override;
        
        // 新スレ作成用のsubbbscgi のURL
        std::string url_subbbscgi_new() const override;

        // datの最大サイズ(Kバイト)
        int get_max_dat_lng() const override { return DEFAULT_MAX_DAT_LNG; }

        // SETTING.TXT
        std::string get_unicode() const override;

      protected:

        // クッキー
        std::string get_hap() const override;
        void set_hap( const std::string& hap ) override;

        // クッキーの更新 (クッキーをセットした時に実行)
        void update_hap() override;

      private:

        // デフォルト最大レス数
        int get_default_number_max_res() const override { return DEFAULT_NUMBER_MAX_2CH; }

        ArticleBase* append_article( const std::string& datbase, const std::string& id, const bool cached ) override;

        void set_cookie_for_be( std::string& cookie ) const;
    };
}

#endif
