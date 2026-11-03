// Command line modes for checking the builder's parts against the Python builder:
//   --xz <in> <out>                 unpacks an .xz file
//   --bindiff <source> <patch> <out>
//   --ips <file> <patch.ips> <out>
//   --unzip <zip> <member> <out>
//   --anim-rebuild <block> <out>
#include "devtools.h"

#include "../build/discwriter.h"
#include "../core/inflate.h"
#include "../core/patches.h"
#include "../core/xz.h"
#include "console.h"

int dev_mode(const std::string &mode, const std::vector<std::string> &a) {
    try {
        auto need = [&](size_t n) {
            if (a.size() != n) throw Error(mode + ": needs " + std::to_string(n) + " arguments");
        };
        if (mode == "--xz") {
            need(2);
            write_file(u8path(a[1]), xz_decompress(read_file(u8path(a[0]))));
        } else if (mode == "--bindiff") {
            need(3);
            write_file(u8path(a[2]), apply_bindiff(read_file(u8path(a[0])), read_file(u8path(a[1]))));
        } else if (mode == "--ips") {
            need(3);
            write_file(u8path(a[2]), apply_ips(read_file(u8path(a[0])), read_file(u8path(a[1]))));
        } else if (mode == "--unzip") {
            need(3);
            ZipArchive z(u8path(a[0]));
            for (auto &e : z.entries())
                if (e.name == a[1]) {
                    z.extract(e, u8path(a[2]));
                    return 0;
                }
            throw Error(a[1] + " not in the zip");
        } else if (mode == "--write-disc") {
            // --write-disc <layout.xml> <out.bin> [unix build time]
            if (a.size() != 2 && a.size() != 3) throw Error(mode + ": needs 2 or 3 arguments");
            write_disc_image(read_text(u8path(a[0])), u8path(a[1]),
                             a.size() == 3 ? std::time_t(std::stoll(a[2])) : std::time(nullptr));
        } else if (mode == "--anim-rebuild") {
            need(2);
            write_file(u8path(a[1]), rebuild_anim_block(read_file(u8path(a[0]))));
        } else {
            return -1;
        }
        return 0;
    } catch (const std::exception &e) {
        console_write(std::string(e.what()) + "\n");
        return 1;
    }
}
