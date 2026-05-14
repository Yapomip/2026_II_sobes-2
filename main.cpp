

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <optional>
#include <ostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

using adjacent = std::vector<size_t>;
using resurce = std::array<size_t, 4>;

struct GameData {
  friend struct Game;

private:
  void parse_room_data_string(std::string s) {
    std::stringstream input(s);
    size_t active_room_index;

    input >> active_room_index;
    size_t room_index;
    do {
      input >> room_index;
      room_adjacent[active_room_index].push_back(room_index);
    } while (input.get() == ',');

    for (auto &res : room_res[active_room_index]) {
      input >> res;
    }
    if (input.fail()) {
      throw std::runtime_error(s);
    }
  }
  void parse_last_string(std::string s) {
    std::stringstream input(s);
    input >> meal;

    std::string res_name;
    input >> res_name;

    if (auto it = std::find(res_names.begin(), res_names.end(), res_name);
        it != res_names.end()) {
      index_double_res = std::distance(res_names.begin(), it);
    } else {
      throw std::runtime_error(s);
    }
    if (input.fail()) {
      throw std::runtime_error(s);
    }
  }

public:
  void parse_game_data(std::istream &input) {
    size_t N;
    input >> N;
    input.get();

    N += 1;

    room_adjacent.resize(N);
    room_res.resize(N);

    for (size_t i = 0; i < N; ++i) {
      std::string buf;
      std::getline(input, buf);
      parse_room_data_string(std::move(buf));
    }
    std::string buf;
    std::getline(input, buf);
    parse_last_string(std::move(buf));
  }
  friend std::ostream &operator<<(std::ostream &out, const GameData &g) {
    out << "N " << g.room_adjacent.size() << " M " << g.meal << std::endl;
    out << "index_double_res " << g.index_double_res << std::endl;
    for (size_t i = 0; i < g.room_adjacent.size(); ++i) {
      for (auto v : g.room_adjacent[i]) {
        out << v << " ";
      }
      out << std::endl << "   ";
      for (auto c : g.room_res[i]) {
        out << c << " ";
      }
      out << std::endl;
    }
    out << g.res_names[g.index_double_res] << std::endl;
    return out;
  }

private:
  constexpr static resurce res_costs = {7, 11, 23, 1};
  constexpr static std::array<std::string_view, 4> res_names = {"iron", "gold",
                                                                "gems", "Exp"};
  std::vector<adjacent> room_adjacent;
  std::vector<resurce> room_res;
  size_t meal;
  size_t index_double_res;
};

struct Game {
private:
  bool is_adjacent_unchecked(size_t from, size_t to) const {
    auto &adj = data.room_adjacent[from];
    return std::find(adj.begin(), adj.end(), to) != adj.end();
  }
  void clear() {
    current_meal = data.meal;
    visited.clear();
    viewed.clear();
    room_res_farmed.clear();

    visited.insert(0);
    std::copy(data.room_adjacent[0].begin(), data.room_adjacent[0].end(),
              viewed);
    room_res_farmed.resize(data.room_res.size(), false);
    current_res = {0, 0, 0, 0};
    current_index = 0;
  }

  size_t compute_farmed_res(size_t res_index) const {
    return data.room_res[current_index][res_index] *
           (res_index == data.index_double_res ? 2 : 1);
  }

  bool spend_meal() {
    if (current_meal > 0) {
      current_meal -= 1;
      return true;
    }
    return false;
  }

public:
  Game(GameData data) : data(std::move(data)) { clear(); }

  bool is_end() { return current_meal == 0; }

  size_t get_current_index() const { return current_index; }
  size_t get_meal() const { return data.meal; }

  bool is_visited(size_t index) const {
    return std::find(visited.begin(), visited.end(), index) != visited.end();
  }
  bool is_viewed(size_t index) const {
    return std::find(viewed.begin(), viewed.end(), index) != viewed.end();
  }
  std::optional<bool> is_adjacent(size_t from, size_t to) const {
    if (is_viewed(from)) {
      return is_adjacent_unchecked(from, to);
    }
    return std::nullopt;
  }
  std::optional<std::reference_wrapper<const adjacent>>
  get_room_adjacents(size_t index) const {
    if (is_viewed(index)) {
      return data.room_adjacent[index];
    }
    return std::nullopt;
  }
  std::optional<std::reference_wrapper<const resurce>>
  get_room_res(size_t index) const {
    if (is_visited(index)) {
      return data.room_res[index];
    }
    return std::nullopt;
  }
  std::optional<std::pair<const adjacent &, const resurce &>> go(size_t index) {
    if (is_adjacent_unchecked(current_index, index) && spend_meal()) {

      visited.insert(index);
      std::copy(data.room_adjacent[index].begin(),
                data.room_adjacent[index].end(), viewed.begin());

      current_index = index;
      return std::make_pair(data.room_adjacent[index], data.room_res[index]);
    }
    return std::nullopt;
  }
  void farm_res(size_t res_index) {
    if (room_res_farmed[current_index]) {
      if (!spend_meal()) {
        return;
      }
    } else {
      room_res_farmed[current_index] = true;
    }
    current_res[res_index] += compute_farmed_res(res_index);
  }

private:
  const GameData data;
  size_t current_meal;
  std::set<size_t> visited;
  std::set<size_t> viewed;
  std::vector<bool> room_res_farmed;
  resurce current_res;
  size_t current_index;
};

struct basic_algorithm {
  Game g;
};

int main(int argc, char *argv[]) {
  GameData g;

  if (argc == 1) {
    g.parse_game_data(std::cin);
  } else {
    std::ifstream input_file(argv[1]);
    if (!input_file) {
      throw std::runtime_error("Input file not exists.");
    }
    g.parse_game_data(input_file);
  }

  std::cout << g;

  return 0;
}
