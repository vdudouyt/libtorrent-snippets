#include <libtorrent/create_torrent.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_metadata.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include <libtorrent/storage.hpp>
#include <libtorrent/file.hpp>
#include <libtorrent/file_pool.hpp>
#include <libtorrent/session.hpp>
#include <cassert>
#include <fstream>
#include <vector>

using namespace std;
using namespace libtorrent;

int calculate_bufs_size(file::iovec_t const* bufs, int num_bufs)
{
   int size = 0;
   for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
   {
      size += i->iov_len;
   }

   return size;
}

class custom_storage : public libtorrent::storage_interface
{
   public:
      custom_storage(file_storage const& fs, file_pool &fp) : m_files(fs), m_pool(fp) {
         myFile.open("2.pdf", ios::out | ios::binary);
      }
      void set_file_priority(std::vector<boost::uint8_t> const&) {}
      bool has_any_file() { return false; }
      bool rename_file(int, std::string const&) { return false; }
      bool initialize(bool) { return false; }
      int move_storage(std::string const&, int) { return 0; }
      int read(char*, int, int, int size) { assert(0); return size; }
      int write(char const*, int, int, int size) { assert(0); return size; }
      size_type physical_offset(int, int) { return 0; }
      int readv(file::iovec_t const* bufs, int slot, int offset, int num_bufs, int flags = file::random_access) { return 0; }
      int writev(file::iovec_t const* bufs, int slot, int offset, int num_bufs, int flags = file::random_access);
      bool move_slot(int, int) { assert(0); return false; }
      bool swap_slots(int, int) { assert(0); return false; }
      bool swap_slots3(int, int, int) { assert(0); return false; }
      bool verify_resume_data(lazy_entry const&, error_code&) { return false; }
      bool write_resume_data(entry&) const { return false; }

      bool release_files()
      {
         cout << "release_files()" << endl;
         return false;
      }

      bool delete_files()
      {
         cout << "delete_files()" << endl;
         return false;
      }
   private:
      file_storage const& m_files;
      file_pool& m_pool;
      ofstream myFile;
};

struct Chunk
{
   Chunk(int file_index, int file_offset, int length, void *data)
      : file_index(file_index), file_offset(file_offset), length(length), data(data) { }
   int file_index;
   int file_offset;
   int length;
   void *data;
};

vector<Chunk> splitFiles(file_storage const& m_files, file::iovec_t const* bufs, int slot, int offset, int num_bufs)
{
   vector<Chunk> ret;

   for (file::iovec_t const* i = bufs, *end(bufs + num_bufs); i < end; ++i)
   {
      int pos = 0;
      size_type start = slot * (size_type)m_files.piece_length() + offset;

      while(pos < i->iov_len)
      {
         int file_index = m_files.file_index_at_offset(start + pos);
         size_type file_offset = start + pos - m_files.file_offset(file_index);
         int bytes_remaining_in_file = m_files.file_size(file_index) - file_offset;
         int bytes_remaining_in_piece = i->iov_len - pos;
   
         if(bytes_remaining_in_file < bytes_remaining_in_piece)
         {
            ret.push_back(Chunk(file_index, file_offset, bytes_remaining_in_file, i->iov_base + pos));
            pos += bytes_remaining_in_file;
         }
         else
         {
            ret.push_back(Chunk(file_index, file_offset, bytes_remaining_in_piece, i->iov_base + pos));
            pos += bytes_remaining_in_piece;
         }
      }

      offset += i->iov_len;
   }

   return ret;
}

int custom_storage::writev(file::iovec_t const* bufs, int slot, int offset, int num_bufs, int flags)
{
   vector<Chunk> chunks = splitFiles(m_files, bufs, slot, offset, num_bufs);
   for(vector<Chunk>::iterator it = chunks.begin(); it != chunks.end(); it++)
   {
      if(it->file_index == 0)
      {
         myFile.seekp(it->file_offset, ios_base::beg);
         myFile.write((const char*) it->data, it->length);
         myFile.flush();
      }
   }
   int size = calculate_bufs_size(bufs, num_bufs);
   return size;
}

storage_interface* custom_storage_constructor(file_storage const& fs
      , file_storage const* mapped, std::string const& path, file_pool& fp
      , std::vector<boost::uint8_t> const& file_prio)
{
   assert(!mapped);
   cout << "path=" << path << endl;
   return new custom_storage(fs, fp);
}

int main(int argc, char **argv)
{
     printf("torrent_file=%s\n", argv[1]);
     session s;
     error_code ec;
     s.listen_on(std::make_pair(6881, 6889), ec);
     if (ec)
     {
        fprintf(stderr, "failed to open listen socket: %s\n", ec.message().c_str());
        return 1;
     }
     add_torrent_params p(custom_storage_constructor);
     p.save_path = "./";
     p.ti = new torrent_info(argv[1], ec);
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
     char a;
     scanf("%c\n", &a);
     return 0;
}
