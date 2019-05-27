#include <iostream>
#include <bitset>
#include <cmath>
#include <ncurses.h>
#include <unistd.h>

using namespace std;

bitset<32> full_adder(bitset<32>, bitset<32>);
bitset<32> addr_parse(const char *);
string addr_parse(bitset<32>);
bitset<32> prefix_get(int prefix);
int get_addr_size(int);

int main(int argc, char *argv[])
{
    static const char *ADDRESS = argv[1];
    static const int PREFIX = atoi(argv[2]);
    static const bitset<32> OFFSET(1);
    static const int USEABLE_ADDR_SIZE = get_addr_size(PREFIX);

    bitset<32> bit = addr_parse(ADDRESS);
    bitset<32> mask = prefix_get(PREFIX);
    bitset<32> addr_network = bit & mask;
    bitset<32> addr_broadcast = addr_network | ~mask;
    bitset<32> addr_current = addr_network | OFFSET;

    WINDOW *pad;
    int line = 0;
    int h = 0;
    int w = 0;
    initscr();
    getmaxyx(stdscr, h, w);
    pad = newpad(USEABLE_ADDR_SIZE, w);

    /**
     * 入力オプション群
     */
    noecho();
    curs_set(0);
    nonl();
    intrflush(stdscr, FALSE);
    intrflush(pad, TRUE);
    keypad(pad, TRUE);

    while (addr_current != addr_broadcast)
    {
        string result = addr_parse(addr_current);
        result = result.substr(0, result.length() - 1);
        mvwprintw(pad, line, 0, "%s", result.data());
        addr_current = full_adder(addr_current, OFFSET);
        line++;
    }
    prefresh(pad, 0, 0, 0, 0, h - 1, w - 1);

    int y = 0;

    while (true)
    {
        int ch = wgetch(pad);

        if (ch == 'q') 
        {
            delwin(pad);
            endwin();
            break;
        }
        if (ch == KEY_DOWN && y + h < USEABLE_ADDR_SIZE)
        {
            y++;
            prefresh(pad, y, 0, 0, 0, h - 1, w - 1);
        }
        else if (ch == KEY_UP && y > 0)
        {
            y--;
            prefresh(pad, y, 0, 0, 0, h - 1, w - 1);
        } else {
            beep();
        }
    }

    return 0;
}

bitset<32> full_adder(bitset<32> a, bitset<32> b)
{
    while (b != 0)
    {
        bitset<32> c = (a & b) << 1;
        a ^= b;
        b = c;
    }
    return a;
}

string addr_parse(bitset<32> address)
{
    int count = 0;
    int temp = 0;
    int n = 7;
    string result = "";
    for (int i = 0; i < 32; i++)
    {
        if (address[31])
        {
            temp += pow(2, n);
        }
        n--;
        count++;
        if ((i + 1) % 8 == 0)
        {
            count = 0;
            result.append(to_string(temp) + ".");
            temp = 0;
            n = 7;
        }
        address <<= 1;
    }

    // return result.substr(0, result.length() - 1).data();
    return result;
}

bitset<32> addr_parse(const char *address)
{
    char octet[4][4];
    int index = 0;
    int index_octet = 0;
    int index_str = 0;

    while (address[index] != '\0')
    {
        if (address[index] == '.')
        {
            octet[index_octet][index_str] = '\0';
            index_octet++;
            index_str = 0;
        }
        else
        {
            octet[index_octet][index_str] = address[index];
            index_str++;
        }
        index++;
    }

    bitset<32> bit;
    int count = 0;
    const unsigned int octet_array[] = {atoi(octet[0]), atoi(octet[1]), atoi(octet[2]), atoi(octet[3])};
    for (int octet : octet_array)
    {
        for (int index = 0; index < 8; index++)
        {
            if (octet % 2 != 0)
            {
                bit.set(index);
            }
            octet /= 2;
        }
        if (count < 3) {
            bit <<= 8;
            count++;
        }
    }
    
    return bit;
}

bitset<32> prefix_get(int prefix)
{
    bitset<32> subnet_mask;
    for (int count = 1; count < prefix; count++)
    {
        subnet_mask.set(31);
        subnet_mask >>= 1;
    }
    subnet_mask.set(31);

    return subnet_mask;
}

int get_addr_size(int prefix)
{
    static const int MAX_PREFIX = 32;

    return pow(2, MAX_PREFIX - prefix) - 2;
}