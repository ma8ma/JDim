// ライセンス: GPL2

//
// ユーザーコマンドの管理クラス
//

#ifndef _USRCMDMANAGER_H
#define _USRCMDMANAGER_H

#include "xml/document.h"

#include <string>
#include <vector>

#define ROOT_NODE_NAME_USRCMD "usrcmdlist"

namespace CORE
{
    class Usrcmd_Manager
    {
        XML::Document m_document;

        int m_size;

        std::vector< std::string > m_list_cmd;
        Glib::RefPtr<Gio::Menu> m_menumodel;
        bool m_need_rebuild{};

    public:

        Usrcmd_Manager();
        virtual ~Usrcmd_Manager() noexcept = default;

        XML::Document& xml_document() { return m_document; }
        void analyze_xml();
        void save_xml();

        int get_size() const { return m_size; }

        // 実行
        void exec( const int comnum, // コマンド番号
                   const std::string& url,
                   const std::string& link,
                   const std::string& selection, // 選択文字
                   const int number // レス番号
            );

        void exec( const std::string& command, // コマンド
                   const std::string& url,
                   const std::string& link,
                   const std::string& selection, // 選択文字
                   const int number // レス番号
            );


        // コマンド置換
        // cmdの$URLをurl, $LINKをlink, $TEXT*をtext, $NUMBERをnumberで置き換えて出力
        // text は UTF-8 であること
        std::string replace_cmd( const std::string& cmd,
                                 const std::string& url,
                                 const std::string& link,
                                 const std::string& text,
                                 const int number ) const;

        // ユーザコマンドメニューの作成
        std::string create_usrcmd_menu( Glib::RefPtr< Gtk::ActionGroup >& action_group );
        std::string create_usrcmd_menu( Glib::RefPtr< Gtk::ActionGroup >& action_group,
                                        const XML::Dom* dom, int& dirno, int& cmdno );

        /// ユーザーコマンドのメニューを初期化して返す
        Glib::RefPtr<Gio::Menu> get_usrcmd_menu();

        Glib::RefPtr< Gtk::Action > get_action( Glib::RefPtr< Gtk::ActionGroup >& action_group, const int num );

        /// ユーザーコマンドのGAction名を返す
        Glib::ustring get_action_name( const int num ) const;

        // 選択不可かどうか判断して visible や sensitive を切り替える
        void toggle_sensitive( Glib::RefPtr< Gtk::ActionGroup >& action_group,
                               const std::string& url_article,
                               const std::string& url_link,
                               const std::string& str_select );

        /// ユーザーコマンドの項目を更新する
        void update_menu_items( const Glib::RefPtr<Gio::SimpleActionGroup>& action_group,
                                const std::string& url_article,
                                const std::string& url_link,
                                const std::string& str_select,
                                const std::string& action_scope );
      private:

        /// ユーザコマンドの登録とメニュー作成
        Glib::RefPtr<Gio::Menu> create_usrcmd_menu( const Glib::ustring& action_scope,
                                                    const XML::Dom* dom, int& cmdno,
                                                    const std::vector<int>& hide );

        bool show_replacetextdiag( std::string& texti, const std::string& title ) const;
        void set_cmd( const std::string& cmd );

        bool is_sensitive( int num, const std::string& link, const std::string& selection ) const;
        bool is_hide( int num, const std::string& url ) const;
    };

    ///////////////////////////////////////
    // インターフェース

    Usrcmd_Manager* get_usrcmd_manager();
    void delete_usrcmd_manager();
}


#endif
