#pragma once
#include <vector>
#include <string>


class CSV {
private:
    std::vector<std::string> _data;
    int _size_x = 0;
    int _size_y = 0;

public:
    CSV();
    CSV(int size_x, int size_y);

    int size_x() const;
    int size_y() const;
    bool is_valid(int x, int y) const;
    std::string& at(int x, int y);
    const std::string& at(int x, int y) const;
    std::string dump() const;

    void add_row();
    void resize(int new_x, int new_y);
};

CSV parse_csv(const std::string& path);
