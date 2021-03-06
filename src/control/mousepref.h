// ライセンス: GPL2

// マウスジェスチャ設定ダイアログ

// MousePref が本体で、MousePrefの各行をダブルクリックすると MouseDiag が開いて個別に操作の設定が出来る
// MouseDiag の各行をダブルクリックすると MouseInputDiag が開いてキー入力が出来る

#ifndef _MOUSEPREFPREF_H
#define _MOUSEPREFPREF_H

#include "mousekeypref.h"
#include "control.h"

namespace CONTROL
{
    //
    // マウスジェスチャ入力をラベルに表示するダイアログ
    //
    class MouseInputDiag : public CONTROL::InputDiag
    {
      public:

        MouseInputDiag( Gtk::Window* parent, const std::string& url, const int id );
    };


    ///////////////////////////////////////


    //
    // 個別のマウスジェスチャ設定ダイアログ
    //
    class MouseDiag : public CONTROL::MouseKeyDiag
    {
      public:

        MouseDiag( Gtk::Window* parent, const std::string& url, const int id, const std::string& str_motions );

      protected:

        std::unique_ptr<InputDiag> create_inputdiag() override;
        std::string get_default_motions( const int id ) override;
        std::vector< int > check_conflict( const int mode, const std::string& str_motion ) override;
    };


    ///////////////////////////////////////


    //
    // マウスジェスチャ設定ダイアログ
    //
    class MousePref : public CONTROL::MouseKeyPref
    {
      public:

        MousePref( Gtk::Window* parent, const std::string& url );

      protected:

        std::unique_ptr<MouseKeyDiag> create_setting_diag( const int id, const std::string& str_motions ) override;
        std::string get_str_motions( const int id ) override;
        std::string get_default_motions( const int id ) override;
        void set_motions( const int id, const std::string& str_motions ) override;
        bool remove_motions( const int id ) override;

      private:

        void slot_cancel_clicked() override;
    };

}

#endif
