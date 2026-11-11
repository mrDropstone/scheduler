#include "csv.hpp"
#include <fstream>
#include <vector>
#include <string>
#include <cassert>


CSV::CSV() {}
CSV::CSV(int size_x, int size_y) : _size_x(size_x), _size_y(size_y), _data(size_x * size_y) {}

int CSV::size_x() const {
    return _size_x;
}
int CSV::size_y() const {
    return _size_y;
}
bool CSV::is_valid(int x, int y) const {
    return x >= 0 && x < size_x() && y >= 0 && y < size_y();
}
std::string& CSV::at(int x, int y) {
    assert(is_valid(x, y));
    return _data.at(size_x() * y + x);
}
const std::string& CSV::at(int x, int y) const {
    assert(is_valid(x, y));
    return _data.at(size_x() * y + x);
}
std::string CSV::dump() const {
    std::string result;
    for (int y = 0; y < size_y(); y++) {
        for (int x = 0; x < size_x(); x++) {
            result += at(x, y);
            if (x + 1 != size_x()) {
                result += ",";
            }
        }
        result += "\n";
    }
    return result;
}

void CSV::add_row() {
    for (int i = 0; i < size_x(); i++) {
        _data.emplace_back();
    }
    _size_y++;
}
void CSV::resize(int new_x, int new_y) {
    std::vector<std::string> new_data;
    for (int y = 0; y < new_y; y++) {
        for (int x = 0; x < new_x; x++) {
            if (is_valid(x, y)) {
                new_data.emplace_back(at(x, y));
            } else {
                new_data.emplace_back();
            }
        }
    }
    _size_x = new_x;
    _size_y = new_y;
    _data.swap(new_data);
}


CSV parse_csv(const std::string& path) {
    std::ifstream source(path, std::ios::binary);
    assert(source.is_open());

    CSV csv;
    std::string line;
    int y = 0;
    while (std::getline(source, line)) {
        csv.add_row();
        int x = 0;
        std::string element;
        for (char c : line) {
            if (c == ',') {
                if (y == 0) {
                    csv.resize(x + 1, csv.size_y());
                }
                assert(csv.is_valid(x, y));
                csv.at(x, y) = std::move(element);
                x++;
            } else {
                element += c;
            }
        }
        if (y == 0) {
            csv.resize(x + 1, csv.size_y());
        }
        assert(csv.is_valid(x, y));
        csv.at(x, y) = std::move(element);
        y++;
    }
    return csv;
}
