#ifndef CS360Utils_H_
#define CS360Utils_H_

#include <stdio.h>
#include <vector>

using namespace std;

class CS360Utils {
    
public:
    CS360Utils();
    ~CS360Utils();
    bool isWhitespace(char c);
    void chomp(char *line);
    char * GetLine(int fds);
    void UpcaseAndReplaceDashWithUnderline(char *str);
    char *FormatHeader(char *str, const char *prefix);
    void GetHeaderLines(vector<char *> &headerLines, int skt, bool envformat);
};

#endif /* CS360_H_ */
