#include <sstream>

/* Pre-process an input stream.
 * Removes things such as comments and replaces them to make things easier for the
 * parser.
*/
std::stringstream preprocess(std::istream& infile) {
    std::stringstream outstream;
    for (int next_char; (next_char = infile.get()) != EOF;) {
        if (next_char == '/' && infile.peek() == '/') {
            while (!infile.eof() && infile.get() != '\n') {}
            outstream.put('\n');
            continue;
        }
        outstream.put(next_char);
    }
    outstream.seekg(0);
    return outstream;
}
