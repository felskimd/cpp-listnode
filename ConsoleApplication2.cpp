#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <unordered_map>

struct ListNode { // ListNode модифицировать нельзя
    ListNode* prev = nullptr; // указатель на предыдущий элемент или nullptr
    ListNode* next = nullptr;
    ListNode* rand = nullptr; // указатель на произвольный элемент данного списка, либо `nullptr` 
    std::string data; // произвольные пользовательские данные 
};

void NonRecursiveDelete(ListNode* head) {
    auto* it = head;
    while (it->next != nullptr) {
        it = it->next;
    }
    while (it != nullptr) {
        auto* prev = it->prev;
        delete it;
        it = prev;
    }
}

void Serialize(const std::string& file_name, ListNode* head) {
    if (head == nullptr) {
        return;
    }
    int current_id = 0;
    std::unordered_map<ListNode*, int> nodes_to_ids;
    nodes_to_ids[nullptr] = -1;
    ListNode* current = head;
    while (current != nullptr) {
        // список с циклом - как-то неправильно (и не даст циклу закончиться)
        if (nodes_to_ids.contains(current)) {
            throw std::runtime_error("infinite list");
        }
        nodes_to_ids[current] = current_id;
        ++current_id;
        current = current->next;
    }
    current = head;
    std::ofstream out(file_name);
    bool first = true;
    while (current != nullptr) {
        if (first) {
            first = false;
        }
        else {
            out << '\n';
        }
        out << current->data << ';' << nodes_to_ids[current->rand];
        current = current->next;
    }
}

struct ParsingData {
    int separator_pos;
    int rand_id;
};

std::optional<ParsingData> ParseNodeEndingLine(std::string& target) {
    // до и после id могут быть пробелы
    static std::regex reg{ "^\\s*-?\\d+\\s*$" };

    int sep_pos = target.size() - 1;
    while (sep_pos >= 0 && target[sep_pos] != ';') {
        --sep_pos;
    }
    if (sep_pos == -1) {
        return std::nullopt;
    }
    auto rand_id_str = target.substr(sep_pos + 1, target.size() - sep_pos);
    if (!std::regex_search(rand_id_str, reg)) {
        return std::nullopt;
    }
    auto rand_id = std::stoi(rand_id_str);
    return ParsingData{.separator_pos = sep_pos, .rand_id = rand_id};
}

ListNode* Deserialize(const std::string& file_name) {
    if (!std::filesystem::exists(file_name)) {
        throw std::runtime_error("file not found");
    }
    ListNode* head = nullptr;
    ListNode* current = nullptr;
    ListNode* prev = nullptr;
    int current_id = 0;
    std::unordered_map<int, int> id_to_rand_value;
    std::unordered_map<int, ListNode*> id_to_node;
    id_to_node[-1] = nullptr;
    std::string node_value;
    std::string line;
    bool first = true;
    std::ifstream in(file_name);
    // можно было сделать проще, но захотелось, чтобы <data> могла быть многострочной
    while (std::getline(in, line)) {
        auto parse_result = ParseNodeEndingLine(line);
        if (parse_result) {
            current = new ListNode;
            current->prev = prev;
            if (first) {
                head = current;
                first = false;
            }
            if (prev) {
                prev->next = current;
            }

            current->data = node_value + line.substr(0, parse_result->separator_pos);
            node_value.clear();
            id_to_node[current_id] = current;
            id_to_rand_value[current_id] = parse_result->rand_id;
            prev = current;
            ++current_id;
        }
        else {
            node_value += line + '\n';
        }
    }
    in.close();

    for (const auto& [from, to] : id_to_rand_value) {
        id_to_node.at(from)->rand = id_to_node.at(to);
    }
    return head;
}

void PrintValues(const ListNode* head) {
    auto* current = head;
    while (current != nullptr) {
        std::cout << current->data << std::endl;
        current = current->next;
    }
}

void CreateRandInput(const std::string& name, size_t lines) {
    if (lines == 0) {
        return;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> len_dist(-1, lines - 1);
    std::uniform_int_distribution<> char_dist(97, 122);
    std::string value = "placeholder";
    std::ofstream out(name);
    bool first = true;
    for (size_t i = 0; i < lines; ++i) {
        if (first) {
            first = false;
        }
        else {
            out << '\n';
        }
        for (char& ch : value) {
            ch = char_dist(gen);
        }
        out << value << ';' << len_dist(gen);
    }
}

int main()
{
    auto* list = Deserialize("inlet.in");
    //PrintValues(list);
    Serialize("outlet.out", list);
    NonRecursiveDelete(list);
    CreateRandInput("rand.txt", 1000000);
    list = Deserialize("rand.txt");
    Serialize("rand_out.txt", list);
    NonRecursiveDelete(list);
}
