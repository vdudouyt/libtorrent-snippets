#include <stdlib.h>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>

int main(int argc, char* argv[])
{
   using namespace libtorrent;

   if (argc != 3)
   {
      fputs("usage: ./simple_client torrent-file output-path\n"
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
   torrent_info *t = new torrent_info(argv[1], ec);
   t->set_real_path(0, argv[2]);
   p.ti = t;
   if (ec)
   {
      fprintf(stderr, "%s\n", ec.message().c_str());
      return 1;
   }
	p.ti;
   s.add_torrent(p, ec);
   if (ec)
   {
      fprintf(stderr, "%s\n", ec.message().c_str());
      return 1;
   }

   // wait for the user to end
   char a;
   scanf("%c\n", &a);
   return 0;
}
