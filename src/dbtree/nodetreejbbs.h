// ライセンス: GPL2

//
// JBBS型ノードツリー
//

#ifndef _NODETREEJBBS_H
#define _NODETREEJBBS_H

#include "nodetreebase.h"

#include <memory>
#include <vector>

namespace JDLIB
{
    class Iconv;
}


namespace DBTREE
{
    class NodeTreeJBBS : public NodeTreeBase
    {
        std::unique_ptr< JDLIB::Iconv > m_iconv;
        std::vector< char > m_decoded_lines;
        int m_mode; // 読み込みモード
        
      public:

        NodeTreeJBBS( const std::string& url, const std::string& date_modified );
        ~NodeTreeJBBS();

      protected:

        void clear() override;
        void init_loading() override;
        void create_loaderdata( JDLIB::LOADERDATA& data ) override;
        char* process_raw_lines( char* rawlines, size_t& size ) override;
        const char* raw2dat( char* rawlines, int& byte ) override;

      private:

        void receive_finish() override;
        void html2dat( char* lines );
    };
}

#endif
