// ライセンス: GPL2
//
// 書き込みなどのプレビュー 
//

#ifndef _ARTICLEVIEWPREVIEW_H
#define _ARTICLEVIEWPREVIEW_H

#include "articleviewbase.h"

namespace ARTICLE
{
    class ArticleViewPreview : public ArticleViewBase
    {
        std::string m_url_messageview;

      public:
        explicit ArticleViewPreview( const std::string& url );
        ~ArticleViewPreview() override;

        bool operate_view( const int control ) override;

      private:

#ifndef USE_GTKMM4
        bool slot_button_press( const std::string& url, int res_number, GdkEventButton* event ) override;
#endif

        void goto_num( const int num_to, const int num_from ) override {}
    };
}

#endif
