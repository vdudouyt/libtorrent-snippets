#include <stdlib.h>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/create_torrent.hpp>

int main(int argc, char* argv[])
{
   using namespace libtorrent;

   if (argc != 3)
   {
      fprintf(stderr, "usage: ./create_torrent real_path virt_path\n");
      return 1;
   }

   file_storage fs;
   fs.add_file("test10.bin", 1024, argv[1]);

   create_torrent t(fs);
   t.add_tracker("http://my.tracker");
   t.set_creator("test test");

   set_piece_hashes(t, ".");
   bencode(std::ostream_iterator<char>(std::cout), t.generate());
}
