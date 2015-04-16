#include <stdlib.h>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>

using namespace libtorrent;

class my_torrent_info : public torrent_info
{
   public:
      my_torrent_info(char* str, boost::system::error_code &ec) : torrent_info(str, ec) { }
      int ssid;
};

int main(int argc, char* argv[])
{

   if (argc != 2)
   {
      fputs("usage: ./simple_client torrent-file\n"
         "to stop the client, press return.\n", stderr);
      return 1;
   }

   session s;
   error_code ec;
   s.listen_on(std::make_pair(6881, 6889), ec);
   if (ec)
   {
      fprintf(stderr, "failed to open listen socket: %s\n", ec.message().c_str());
      return 1;
   }
   add_torrent_params p;
   p.save_path = "./";
   my_torrent_info *my_ti = new my_torrent_info(argv[1], ec);
   my_ti->ssid = 123456;
   p.ti = my_ti;
   if (ec)
   {
      fprintf(stderr, "%s\n", ec.message().c_str());
      return 1;
   }
   s.add_torrent(p, ec);
   if (ec)
   {
      fprintf(stderr, "%s\n", ec.message().c_str());
      return 1;
   }

   // wait for the user to end
   while(1)
   {
      std::vector<torrent_handle> handles = s.get_torrents();
      torrent_handle th = handles[0];
      const torrent_info &ti = th.get_torrent_info();
      const my_torrent_info &xs = static_cast<const my_torrent_info&>(ti);
      std::cout << xs.ssid << std::endl;
      sleep(1000);
   }
}
