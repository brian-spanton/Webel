// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Types.h"

namespace Gzip
{
    using namespace Basic;

    enum CompressionMethod : byte
    {
        cm_deflate = 8,
    };

    struct Flags
    {
        byte FTEXT : 1;
        byte FHCRC : 1;
        byte FEXTRA : 1;
        byte FNAME : 1;
        byte FCOMMENT : 1;
        byte reserved : 3;
    };

    enum DeflateFlags : byte
    {
        df_smallest = 2,
        df_fastest = 4,
    };

    enum OperatingSystems : byte
    {
        os_fat = 0, // FAT filesystem(MS - DOS, OS / 2, NT / Win32)
        os_amiga = 1, // Amiga
        os_vms = 2, // VMS (or OpenVMS)
        os_unix = 3, // Unix
        os_vm_cms = 4, // VM / CMS
        os_atari_tos = 5, // Atari TOS
        os_hpfs = 6, // HPFS filesystem(OS / 2, NT)
        os_macintosh = 7, // Macintosh
        os_z_system = 8, // Z - System
        os_cp_m = 9, // CP / M
        os_tops_20 = 10, // TOPS - 20
        os_ntfs = 11, // NTFS filesystem(NT)
        os_qdos = 12, // QDOS
        os_acorn_riscos = 13, // Acorn RISCOS
        os_unknown = 255, // unknown
    };

    struct Subfield
    {
        byte SI1;
        byte SI2;
        uint16 LEN;
        String<byte> data;
    };

    struct MemberHeader
    {
        byte ID1;
        byte ID2;
        CompressionMethod CM;
        Flags FLG;
        uint32 MTIME;
        DeflateFlags XFL;
        OperatingSystems OS;
        byte XLEN;
        uint16 CRC16;
        std::shared_ptr<IStream<byte> > output;
        uint32 CRC32;
        uint32 ISIZE;
        std::vector<Subfield> extra_field;
        String<Codepoint> original_file_name;
        String<Codepoint> file_comment;
    };

    struct ExtraBits
    {
        byte length;
        uint16 base;
    };

    template <typename value_type>
    struct HuffmanAlphabet
    {
        std::shared_ptr<HuffmanAlphabet<value_type> > children[2];

        value_type symbol;

        bool is_leaf()
        {
            if (!children[0] != !children[1])
                throw FatalError("bad huffman tree with non-leaf unbalanced node");

            return !children[0] && !children[1];
        }

        template <typename length_type>
        static void make_alphabet(byte max_length, uint16 count, std::vector<length_type>& lengths, std::shared_ptr<HuffmanAlphabet<value_type> >* alphabet)
        {
            if (max_length > 15)
                throw FatalError("Gzip::HuffmanAlphabet::make_alphabet max_length > 15");

            std::vector<byte> length_count;
            length_count.insert(length_count.end(), max_length + 1, 0);

            for (uint16 i = 0; i < count; i++)
            {
                auto length = lengths[i];
                if (length == 0)
                    continue;

                length_count[length]++;
            }

            std::vector<uint16> next_code;
            next_code.insert(next_code.end(), max_length + 1, 0);

            uint16 code = 0;
            for (byte length = 1; length <= max_length; length++)
            {
                code = (code + length_count[length - 1]) << 1;
                next_code[length] = code;
            }

            auto root = std::make_shared<HuffmanAlphabet<value_type> >();

            for (uint16 symbol = 0; symbol < count; symbol++)
            {
                auto length = lengths[symbol];
                if (length == 0)
                    continue;

                auto code = next_code[length];
                next_code[length]++;

                auto current = root;

                for (byte bit_index = 0; bit_index < length; bit_index++)
                {
                    byte shift = length - bit_index - 1;
                    byte bit = (code >> shift) & 1;

                    if (!current->children[bit])
                        current->children[bit] = std::make_shared<HuffmanAlphabet<value_type> >();

                    current = current->children[bit];
                }

                current->symbol = (value_type)symbol;
            }

            (*alphabet) = root;
        }
    };
}