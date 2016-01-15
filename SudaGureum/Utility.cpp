#include "Common.h"

#include "Utility.h"

namespace SudaGureum
{
    std::string encodeUtf8(const std::wstring &str)
    {
#if 0
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(str);
#endif

        return boost::locale::conv::utf_to_utf<char>(str);
    }

    std::wstring decodeUtf8(const std::string &str)
    {
#if 0
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(str);
#endif

        return boost::locale::conv::utf_to_utf<wchar_t>(str);
    }

    // Base64 from http://stackoverflow.com/questions/10521581/base64-encode-using-boost-throw-exception

    template<typename IterT>
    using ToBase64Iterator = boost::archive::iterators::base64_from_binary<
        boost::archive::iterators::transform_width<IterT, 6, 8>>;
    template<typename IterT>
    using FromBase64Iterator = boost::archive::iterators::transform_width<
        boost::archive::iterators::binary_from_base64<IterT>, 8, 6>;

    std::string encodeBase64(const std::vector<uint8_t> &data)
    {
        size_t padLen = (3 - data.size() % 3) % 3;
        std::string encoded(ToBase64Iterator<std::vector<uint8_t>::const_iterator>(data.begin()), ToBase64Iterator<std::vector<uint8_t>::const_iterator>(data.end()));
        encoded.append(padLen, '=');
        return encoded;
    }

    std::string encodeBase64(boost::any_range<uint8_t, boost::single_pass_traversal_tag> data)
    {
        typedef decltype(data) RangeT;

        std::string encoded(ToBase64Iterator<RangeT::const_iterator>(data.begin()), ToBase64Iterator<RangeT::const_iterator>(data.end()));
        size_t padLen = (4 - encoded.size() % 4) % 4;
        encoded.append(padLen, '=');
        return encoded;
    }

    std::vector<uint8_t> decodeBase64(std::string encoded)
    {
        if(encoded.empty())
        {
            return std::vector<uint8_t>();
        }
        else if(encoded.size() < 4)
        {
            throw(std::invalid_argument("base64 string is too short"));
            return std::vector<uint8_t>();
        }

        size_t padLen = std::count(encoded.end() - 2, encoded.end(), '=');
        std::replace(encoded.end() - padLen, encoded.end(), '=', 'A'); // '\0'
        std::vector<uint8_t> decoded(FromBase64Iterator<std::string::const_iterator>(encoded.begin()), FromBase64Iterator<std::string::const_iterator>(encoded.end()));
        decoded.erase(decoded.end() - padLen, decoded.end());
        return decoded;
    }

    std::array<uint8_t, 20> hashSha1(const std::vector<uint8_t> &data)
    {
        std::array<uint8_t, 20> result;
        SHA1(data.data(), data.size(), result.data());
        return result;
    }

    std::string hashSha1ToHexString(const std::vector<uint8_t> &data)
    {
        static const char HexDigits[] =
        {
            '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
        };

        auto digest = hashSha1(data);

        std::string str;
        for(uint8_t b: digest)
        {
            str += HexDigits[(b & 0xF0) >> 4];
            str += HexDigits[b & 0x0F];
        }

        return str;
    }

    std::deque<uint8_t> readFile(const boost::filesystem::path &path)
    {
        typedef boost::filesystem::basic_ifstream<uint8_t> bifstream;

        bifstream ifp(path, bifstream::binary);
        if(!ifp)
        {
            throw(std::runtime_error("cannot open the file"));
            return {};
        }

        std::deque<uint8_t> data;
        std::array<uint8_t, BUFSIZ> chunk;
        while(ifp.read(chunk.data(), chunk.size()) || ifp.gcount())
        {
            data.insert(data.end(), chunk.begin(), chunk.begin() + ifp.gcount());
        }

        return data;
    }

    std::vector<uint8_t> readFileIntoVector(const boost::filesystem::path &path)
    {
        auto data = readFile(path);
        std::vector<uint8_t> vec(data.begin(), data.end());
        return vec;
    }
}
