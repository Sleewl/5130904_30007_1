#include "iterator.h"
#include <sstream>

namespace gruzdev
{
    StreamGuard::StreamGuard(std::basic_ios<char>& s) :
        s_(s),
        fill_(s.fill()),
        precision_(s.precision()),
        fmt_(s.flags())
    {}

    StreamGuard::~StreamGuard()
    {
        s_.fill(fill_);
        s_.precision(precision_);
        s_.flags(fmt_);
    }

    std::istream& operator>>(std::istream& in, DelimiterIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry)
        {
            return in;
        }
        char c = '0';
        in >> c;
        if (in && (c != dest.exp))
        {
            in.setstate(std::ios::failbit);
        }
        return in;
    }

    std::istream& operator>>(std::istream& in, DoubleIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry)
        {
            return in;
        }
        std::string input = "";
        in >> input;
        if ((input[1] == '.' && (input[4] == 'e'  input[4] == 'E')
            && (input[5] == '+'  input[5] == '-'))
             (input[1] == '.' && (input[3] == 'e'  input[3] == 'E')
                && (input[4] == '+' || input[4] == '-')))
        {
            if (input[input.length() - 2] == ':')
            {
                in.putback(')');
                in.putback(':');
                input.erase(input.length() - 2, input.length());
                dest.ref = std::stod(input);
            }
            else
            {
                in.putback(*input.rbegin());
                in.putback('y');
                in.putback('e');
                in.putback('k');
                in.putback(':');
                input.erase(input.length() - 5, input.length());
                dest.ref = std::stod(input);
            }
        }
        else
        {
            in.setstate(std::ios::failbit);
        }
        return in;
    }

    std::istream& operator>>(std::istream& in, CharIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry)
        {
            return in;
        }
        char c;
        if (in >> DelimiterIO{ '\'' } >> c >> DelimiterIO{ '\'' })
        {
            dest.ref = c;
        }
        return in;
    }

    std::istream& operator>>(std::istream& in, StringIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry)
        {
            return in;
        }
        return std::getline(in >> DelimiterIO{ '"' }, dest.ref, '"');
    }

    std::istream& operator>>(std::istream& in, UllLitIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry)
        {
            return in;
        }
        std::string input;
        in >> input;
        if (input.back() == ':')
        {
            input.pop_back();
        }
        dest.ref = std::stoull(input);
        return in;
    }

    std::istream& operator>>(std::istream& in, UllOctIO&& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry)
        {
            return in;
        }
        std::string input;
        in >> input;
        if (input.back() == ':')
        {
            input.pop_back();
        }
        dest.ref = std::stoull(input, nullptr, 8);
        return in;
    }

    std::istream& operator>>(std::istream& in, DataStruct& dest)
    {
        std::istream::sentry sentry(in);
        if (!sentry)
        {
            return in;
        }

        DataStruct input;
        {
            using sep = DelimiterIO;
            using dbl = DoubleIO;
            using chr = CharIO;
            using str = StringIO;
            using ull_lit = UllLitIO; // Добавлен новый тип для поля key1
            using ull_oct = UllOctIO; // Добавлен новый тип для поля key2
            in >> sep{ '(' };
            bool flag1 = false, flag2 = false, flag3 = false;
            while (true)
            {
                if (flag1 && flag2 && flag3)
                {
                    break;
                }
                std::string key;
                char c = '0';
                in >> c;
                if (!in)
                {
                    break;
                }

                if (c == ':' && (in >> key))
                {
                    if (key == "key1")
                    {
                        in >> ull_lit{ input.key1 };
                        flag1 = true;
                    }
                    else if (key == "key2")
                    {
                        in >> ull_oct{ input.key2 };
                        flag2 = true;
                    }
                    else if (key == "key3")
                    {
                        in >> str{ input.key3 };
                        flag3 = true;
                    }
                }
            }
            in >> sep{ ':' } >> sep{ ')' };
        }
        if (in)
        {
            dest = input;
        }
        return in;
    }

    std::string doubleToScientific(double x)
    {
        std::stringstream ss;
        ss << std::scientific << x;
        std::string out = ss.str();
        size_t i = std::min(out.find('E'), out.find('e'));
        while (out[i - 1] == '0' && out[i - 2] != '.')
        {
            out.erase(i - 1, 1);
            i = std::min(out.find('E'), out.find('e'));
        }
        while (out[i + 2] == '0')
        {
            out.erase(i + 2, 1);
            i = std::min(out.find('E'), out.find('e'));
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const DataStruct& src)
    {
        std::ostream::sentry sentry(out);

        if (!sentry)
        {
            return out;
        }

        StreamGuard fmtguard(out);
        out << "(";
        out << ":key1 " << src.key1; // Записываем без форматирования, т.к. это unsigned long long
        out << ":key2 " << src.key2; // Записываем без форматирования, т.к. это unsigned long long
        out << ":key3 " << "\"" << src.key3 << "\"" << ":";
        out << ")";
        return out;
    }
}
