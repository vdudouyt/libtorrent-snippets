#include <stdlib.h>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>

int main(int argc, char* argv[])
{
   using namespace libtorrent;

   if (argc != 2)
   {
      fputs("usage: ./simple_client torrent-file output-path\n"
         "to stop the client, press return.\n", stderr);
      return 1;
   }

   session s;
}
