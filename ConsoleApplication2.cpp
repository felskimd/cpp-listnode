#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>

struct ListNode { // ListNode модифицировать нельзя
    ListNode* prev = nullptr; // указатель на предыдущий элемент или nullptr
    ListNode* next = nullptr;
    ListNode* rand = nullptr; // указатель на произвольный элемент данного списка, либо `nullptr` 
    std::string data; // произвольные пользовательские данные 
};

//Реализуйте сериализацию и десериализацию двусвязного списка ListNode, где:
//-prev и next указывают на предыдущий / следующий элемент списка
//- rand указывает на произвольный элемент этого же списка или nullptr
//- Каждый элемент содержит строку data
//Необходимо :
//1. Считать текстовое описание списка из файла inlet.in
//2. Построить связанный список ListNode* head по этим данным
//3. Сериализовать список в бинарный файл outlet.out

//2. Формат входного файла inlet.in
//Простой текстовый формат.Каждая строка описывает один узел :
//<data>; <rand_index>
//Где:
//-<data> — строка(могут быть пробелы, спецсимволы, кодировка UTF - 8)
//- <rand_index> — индекс узла, на который указывает rand, либо - 1, если rand == nullptr
//Пример входного файла(inlet.in) :
//    apple; 2
//    banana; -1
//    carrot; 1
//    Этот список :
//-Узел 0 : "apple" → rand на узел 2
//- Узел 1 : "banana" → rand = nullptr
//- Узел 2 : "carrot" → rand на узел 1

//3. Выходной файл(outlet.out)
//Бинарный файл, содержащий сериализованное представление двусвязного списка.
//4. Ограничения
//- Максимальное число узлов : 10⁶
//- data может быть длиной до 1000 символов
//6. Требования к сдаче работы
//Работы принимаются только в виде ссылки на публичный репозиторий на GitHub
//Требования к ссылке :
//·	Должна быть указана ссылка на сам репозиторий, а не на архив, отдельный файл или ветку.
//·	Репозиторий должен быть доступен для клонирования командой git clone <ссылка>
void RecursiveDelete(ListNode* head) {
    if (head->next) {
        RecursiveDelete(head->next);
    }
    delete head;
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
        if (nodes_to_ids.contains(current)) { // список с циклом - как-то неправильно (и не даст циклу закончиться)
            throw std::runtime_error("infinite list");
        }
        nodes_to_ids[current] = current_id;
        ++current_id;
        current = current->next;
    }
    current = head;
    std::ofstream out(file_name);
    while (current != nullptr) {
        out << current->data << ';' << nodes_to_ids[current->rand] << '\n';
        current = current->next;
    }
}

struct ParsingData {
    int separator_pos;
    int rand_id;
};

std::optional<ParsingData> ParseNodeEndingLine(std::string& target) {
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
    int current_id = 0;
    std::unordered_map<int, int> id_to_rand_value;
    ListNode* prev = nullptr;
    std::unordered_map<int, ListNode*> id_to_node;
    id_to_node[-1] = nullptr;
    std::string node_value;
    std::string line;
    bool first = true;
    std::ifstream in(file_name);
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

int main()
{
    auto* list = Deserialize("inlet.in");
    PrintValues(list);
    Serialize("outlet.out", list);
    RecursiveDelete(list);
}
