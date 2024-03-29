// Copyright � 2013 Brian Spanton

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
        ByteString data;
    };

    struct MemberHeader
    {
        byte ID1 = 0;
        byte ID2 = 0;
        CompressionMethod CM = CompressionMethod::cm_deflate;
        Flags FLG = { 0 };
        uint32 MTIME = 0;
        DeflateFlags XFL = DeflateFlags::df_smallest;
        OperatingSystems OS = OperatingSystems::os_unknown;
        byte XLEN = 0;
        uint16 CRC16 = 0;
        std::shared_ptr<IStream<byte> > output;
        uint32 CRC32 = 0;
        uint32 ISIZE = 0;
        std::vector<Subfield> extra_field;
        String<Codepoint> original_file_name;
        String<Codepoint> file_comment;
    };

    struct ExtraBits
    {
        byte length;
        uint16 base;
    };

    template <typename symbol_type>
    struct HuffmanAlphabet
    {
        std::shared_ptr<HuffmanAlphabet<symbol_type> > children[2];

        uint16 code = 0;
        byte length = 0;
        symbol_type symbol = 0;

        bool validate()
        {
            return validate(0, 0);
        }

        bool validate(byte test_length, uint16 test_code)
        {
            if (children[0])
            {
                if (!children[1])
                {
                    // there is one specific edge case with Deflate that defies this rule
                    Basic::LogDebug("Gzip", "HuffmanAlphabet", "validate", "!children[1] (non-leaf nodes must have both children)");
                    return false;
                }

                if (!children[0]->validate(test_length + 1, (test_code << 1) | 0))
                    return false;

                if (!children[1]->validate(test_length + 1, (test_code << 1) | 1))
                    return false;

                return true;
            }

            if (children[1])
            {
                Basic::LogDebug("Gzip", "HuffmanAlphabet", "validate", "children[1] (non-leaf nodes must have both children)");
                return false;
            }

            // it is a leaf node, validate the node values

            if (this->code != test_code)
            {
                Basic::LogDebug("Gzip", "HuffmanAlphabet", "validate", "this->code != test_code");
                return false;
            }

            if (this->length != test_length)
            {
                Basic::LogDebug("Gzip", "HuffmanAlphabet", "validate", "this->length != test_length");
                return false;
            }

            return true;
        }

        bool is_leaf()
        {
            return !children[0] && !children[1];
        }

        static bool make_alphabet(uint16 count, std::vector<byte>& lengths, std::shared_ptr<HuffmanAlphabet<symbol_type> >* alphabet)
        {
            if (count > lengths.size())
            {
                Basic::LogDebug("Gzip", "HuffmanAlphabet", "make_alphabet", "count > lengths.size()");
                return false;
            }

            std::vector<uint16> length_count;
            length_count.reserve(16);

            for (uint16 symbol = 0; symbol < count; symbol++)
            {
                byte length = lengths[symbol];
                if (length == 0)
                    continue;

                byte length_vector_size = length + 1;
                if (length_vector_size > length_count.size())
                    length_count.insert(length_count.end(), length_vector_size - length_count.size(), 0);

                length_count[length]++;

                if (length_count[length] > (1 << length))
                {
                    Basic::LogDebug("Gzip", "HuffmanAlphabet", "make_alphabet", "first_code > masks[length]");
                    return false;
                }
            }

            if (length_count.size() > 16)
            {
                Basic::LogDebug("Gzip", "HuffmanAlphabet", "make_alphabet", "length_count.size() > 16");
                return false;
            }

            byte max_length = (byte)length_count.size() - 1;

            std::vector<uint16> next_code;
            next_code.insert(next_code.end(), max_length + 1, 0);

            uint16 first_code = 0;
            for (byte length = 1; length <= max_length; length++)
            {
                uint16 previous_count = length_count[length - 1];
                uint16 previous_last_code = first_code + previous_count;
                first_code = (previous_last_code << 1);

                if (first_code > ((1 << length) - 1))
                {
                    Basic::LogDebug("Gzip", "HuffmanAlphabet", "make_alphabet", "first_code > ((1 << length) - 1)");
                    return false;
                }

                next_code[length] = first_code;
            }

            auto root = std::make_shared<HuffmanAlphabet<symbol_type> >();

            for (uint16 symbol = 0; symbol < count; symbol++)
            {
                byte length = lengths[symbol];
                if (length == 0)
                    continue;

                uint16 code = next_code[length];
                next_code[length]++;

                auto current = root;

                for (byte bit_index = 0; bit_index < length; bit_index++)
                {
                    byte shift = length - bit_index - 1;
                    byte bit = (code >> shift) & 1;

                    if (!current->children[bit])
                        current->children[bit] = std::make_shared<HuffmanAlphabet<symbol_type> >();

                    current = current->children[bit];
                }

                if (current->length != 0)
                {
                    Basic::LogDebug("Gzip", "HuffmanAlphabet", "make_alphabet", "duplicate huffman code");
                    return false;
                }

                current->symbol = (symbol_type)symbol;
                current->code = code;
                current->length = length;
            }

            if (!root->validate())
                return false;

            (*alphabet) = root;
            return true;
        }
    };
}