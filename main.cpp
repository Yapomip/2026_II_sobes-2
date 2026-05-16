

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <map>
#include <numeric>
#include <optional>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using adjacent = std::vector<size_t>;
using resurce = std::array<size_t, 4>;

struct GameData {
  friend struct Game;

private:
  size_t parse_first_string(std::string s) {
    std::stringstream input(s);
    size_t N;
    input >> N;
    if (input.fail() || !input.eof()) {
      throw std::runtime_error(s);
    }
    return N;
  }
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
    if (input.fail() || !input.eof()) {
      throw std::runtime_error(s);
    }
  }
  void parse_last_string(std::string s) {
    std::stringstream input(s);
    input >> meal;

    std::string name;
    input >> name;

    if (auto it = std::find(res_name.begin(), res_name.end(), name);
        it != res_name.end()) {
      index_double_res = std::distance(res_name.begin(), it);
    } else {
      throw std::runtime_error(s);
    }
    if (input.fail() || !input.eof()) {
      throw std::runtime_error(s);
    }
  }

public:
  void parse_game_data(std::istream &input) {
    std::string buf;
    std::getline(input, buf);
    size_t N = parse_first_string(std::move(buf));
    N += 1;

    room_adjacent.resize(N);
    room_res.resize(N);

    for (size_t i = 0; i < N; ++i) {
      std::getline(input, buf);
      parse_room_data_string(std::move(buf));
    }
    std::getline(input, buf);
    parse_last_string(std::move(buf));
  }
  friend std::ostream &operator<<(std::ostream &out, const GameData &gd) {
    out << "N " << gd.room_adjacent.size() << " M " << gd.meal << std::endl;
    out << "index_double_res " << gd.index_double_res << std::endl;
    for (size_t i = 0; i < gd.room_adjacent.size(); ++i) {
      for (auto v : gd.room_adjacent[i]) {
        out << v << " ";
      }
      out << std::endl << "   ";
      for (auto c : gd.room_res[i]) {
        out << c << " ";
      }
      out << std::endl;
    }
    out << gd.res_name[gd.index_double_res] << std::endl;
    return out;
  }

private:
  constexpr static resurce res_cost = {7, 11, 23, 1};
  constexpr static std::array<std::string_view, 4> res_name = {"iron", "gold",
                                                               "gems", "Exp"};
  constexpr static resurce res_index_in_order = {2, 1, 0, 3};
  std::vector<adjacent> room_adjacent;
  std::vector<resurce> room_res;
  size_t meal;
  size_t index_double_res;
};

struct Game {
private:
  bool is_adjacent_unchecked(size_t from, size_t to) const {
    auto &adj = room_adjacent[from];
    return std::find(adj.begin(), adj.end(), to) != adj.end();
  }

  bool spend_meal() {
    if (current_meal > 0) {
      current_meal -= 1;
      return true;
    }
    return false;
  }

public:
  Game(GameData data)
      : room_adjacent(std::move(data.room_adjacent)),
        room_res(std::move(data.room_res)), res_cost(std::move(data.res_cost)),
        res_name(std::move(data.res_name)), res_index_in_order(),
        current_index(0), current_res({0, 0, 0, 0}), current_meal(data.meal),
        farmed_room_res(room_res.size(), false), viewed(), visited() {
    res_cost[data.index_double_res] *= 2;
    std::iota(res_index_in_order.begin(), res_index_in_order.end(), 0);
    std::sort(res_index_in_order.begin(), res_index_in_order.end(),
              [this](size_t i, size_t j) { return res_cost[i] > res_cost[j]; });

    visited.insert(0);
    std::copy(room_adjacent[0].begin(), room_adjacent[0].end(),
              std::inserter(viewed, viewed.begin()));
    for (size_t i = 0; i < room_adjacent.size(); ++i) {
      for (auto j : room_adjacent[i]) {
        if (std::find(room_adjacent[j].begin(), room_adjacent[j].end(), i) ==
            room_adjacent[j].end()) {
          room_adjacent[j].push_back(i);
        }
      }
    }

    for (auto &adj : room_adjacent) {
      std::sort(adj.begin(), adj.end());
    }
  }
  friend std::ostream &operator<<(std::ostream &out, const Game &g) {
    out << "index: " << g.current_index << " meal " << g.current_meal
        << std::endl
        << "res: ";
    for (size_t i = 0; i < g.current_res.size(); ++i) {
      out << g.current_res[i] << " ";
    }
    out << std::endl;
    for (size_t i = 0; i < g.res_cost.size(); ++i) {
      out << g.res_name[i] << " " << g.res_cost[i] << std::endl;
    }
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
    return out;
  }

  bool is_end() const { return current_meal == 0; }
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

  size_t get_current_room_index() const { return current_index; }
  size_t get_meal() const { return current_meal; }
  const resurce &get_current_res() const { return current_res; }
  const std::set<size_t> get_visited() const { return visited; }
  const std::set<size_t> get_viewed() const { return viewed; }
  const adjacent &get_current_room_adjacents() const {
    return room_adjacent[current_index];
  }
  const resurce &get_current_room_res() const {
    return room_res[current_index];
  }
  std::optional<std::reference_wrapper<const adjacent>>
  get_room_adjacents(size_t index) const {
    if (is_viewed(index)) {
      return room_adjacent[index];
    }
    return std::nullopt;
  }
  std::optional<std::reference_wrapper<const resurce>>
  get_room_res(size_t index) const {
    if (is_visited(index)) {
      return room_res[index];
    }
    return std::nullopt;
  }
  std::string_view get_res_name(size_t res_index) const {
    return res_name[res_index];
  }
  const resurce &get_res_cost() const { return res_cost; }
  const std::array<size_t, 4> &get_res_index_in_order() const {
    return res_index_in_order;
  }

  std::optional<std::pair<const adjacent &, const resurce &>> go(size_t index) {
    if (is_adjacent_unchecked(current_index, index) && spend_meal()) {
      visited.insert(index);
      std::copy(room_adjacent[index].begin(), room_adjacent[index].end(),
                std::insert_iterator(viewed, viewed.begin()));

      current_index = index;
      return std::make_pair(room_adjacent[index], room_res[index]);
    }
    return std::nullopt;
  }
  void farm_res(size_t res_index) {
    if (farmed_room_res[current_index]) {
      if (!spend_meal()) {
        return;
      }
    } else {
      farmed_room_res[current_index] = true;
    }
    current_res[res_index] += 1;
  }

private:
  std::vector<adjacent> room_adjacent;
  std::vector<resurce> room_res;

  resurce res_cost;
  std::array<std::string_view, 4> res_name;
  resurce res_index_in_order;

  size_t current_index;
  resurce current_res;
  size_t current_meal;

  std::set<size_t> visited;
  std::set<size_t> viewed;
  std::vector<bool> farmed_room_res;
};

struct BasicPlayerAlgorithm {
private:
  void go(Game &g, size_t index) {
    g.go(index);

    out << "go " << index << std::endl;
    if (index != 0) {
      out << "state " << index;
      for (auto r : g.get_current_res()) {
        out << " " << r;
      }
      out << std::endl;
    }
  }
  void go_next_room(Game &g) {
    size_t index = 0;

    auto &adjacent = g.get_current_room_adjacents();
    auto &vizited = g.get_visited();
    for (auto adj : adjacent) {
      if (vizited.find(adj) == vizited.end()) {
        index = adj;
        break;
      }
    }
    if (index == 0) {
      auto &viewed = g.get_viewed();
      std::set<size_t> viewed_not_vizited;

      std::set_difference(
          viewed.begin(), viewed.end(), vizited.begin(), vizited.end(),
          std::inserter(viewed_not_vizited, viewed_not_vizited.begin()));

      std::optional<std::vector<size_t>> best_path;
      auto graph = create_graph(g);
      for (auto not_vizited_room : viewed_not_vizited) {
        auto path =
            find_path(graph, g.get_current_room_index(), not_vizited_room)
                .value();
        if (!best_path || path.size() < best_path->size()) {
          best_path = std::move(path);
        }
      }
      if (!best_path) {
        return;
      }
      index = best_path.value()[1];
    }

    go(g, index);
  }
  void farm_res(Game &g, size_t res_index) {
    g.farm_res(res_index);

    out << "collect " << g.get_res_name(res_index) << std::endl;
    out << "state " << g.get_current_room_index();
    auto res = g.get_current_res();
    for (size_t i = 0; i < res.size(); ++i) {
      if (i == res_index) {
        out << " _";
      } else {
        out << " " << res[i];
      }
    }
    out << std::endl;
  }
  static std::optional<size_t> get_max_res_index(Game &g) {
    auto &room_res = g.get_current_room_res();
    auto &res_index_in_order = g.get_res_index_in_order();
    for (auto i : res_index_in_order) {
      if (room_res[i] > 0) {
        return i;
      }
    }
    return std::nullopt;
  }
  void farm_res_max(Game &g) {
    if (auto index = get_max_res_index(g); index) {
      farm_res(g, index.value());
    }
  }
  size_t farm_res_n(Game &g, size_t n) {
    if (n == 0) {
      return 0;
    }
    size_t i = 0;
    for (auto index = get_max_res_index(g); index && i < n;
         index = get_max_res_index(g)) {
      farm_res(g, index.value());
      i += 1;
    }
    return i;
  }
  static std::optional<std::vector<size_t>>
  find_path(const std::map<size_t, const adjacent &> &graph, size_t begin,
            size_t end) {
    std::set<size_t> visited({begin});
    std::map<size_t, size_t> reversed_path({{end, begin}});
    std::queue<size_t> q({begin});

    while (!q.empty()) {
      auto current = q.front();
      q.pop();

      if (current == end) {
        size_t prev = end;
        std::vector<size_t> path({end});
        while (prev != begin) {
          prev = reversed_path[path.back()];
          path.push_back(prev);
        }
        std::reverse(path.begin(), path.end());
        return path;
      }

      auto &adj = graph.find(current)->second;
      for (auto neighbor : adj) {
        if (graph.count(neighbor) && !visited.count(neighbor)) {
          visited.insert(neighbor);
          reversed_path[neighbor] = current;
          q.push(neighbor);
        }
      }
    }
    return std::nullopt;
  }
  static std::map<size_t, const adjacent &> create_graph(Game &g) {
    auto &graph_vertex = g.get_visited();
    std::map<size_t, const adjacent &> graph;
    for (auto &vertex : graph_vertex) {
      auto &adj = g.get_room_adjacents(vertex).value().get();
      graph.insert({vertex, adj});
    }
    return graph;
  }
  static std::optional<std::vector<size_t>> find_path_to(Game &g, size_t to) {
    auto graph = create_graph(g);
    return find_path(graph, g.get_current_room_index(), to);
  }

public:
  BasicPlayerAlgorithm(std::ostream &out) : out(out) {}
  void play(Game &g) {
    auto meal = g.get_meal();
    for (size_t i = 0; i < meal / 2; ++i) {
      go_next_room(g);
      farm_res_max(g);
    }
    auto path = find_path_to(g, 0).value();
    for (size_t i = 1; i < path.size(); ++i) {
      if (g.get_meal() > path.size() - 1) {
        farm_res_n(g, g.get_meal() - path.size() + 1);
      }
      go(g, path[i]);
    }

    size_t final_res_cost = 0;
    auto &res = g.get_current_res();
    auto &res_cost = g.get_res_cost();
    for (size_t i = 0; i < res.size(); ++i) {
      final_res_cost += res[i] * res_cost[i];
    }
    out << "result";
    for (size_t i = 0; i < res.size(); ++i) {
      out << " " << res[i];
    }
    out << " " << final_res_cost;
  }

private:
  std::ostream &out;
};

int main(int argc, char *argv[]) {
  GameData game_data;

  if (argc == 1) {
    game_data.parse_game_data(std::cin);

  } else {
    std::ifstream input_file(argv[1]);
    if (!input_file) {
      throw std::runtime_error("Input file not exists.");
    }
    game_data.parse_game_data(input_file);
  }

  // std::cout << game_data;
  Game game(std::move(game_data));
  if (argc == 1) {
    BasicPlayerAlgorithm bp(std::cout);
    bp.play(game);
  } else {
  std:
    std::ofstream result_file("result.txt");
    BasicPlayerAlgorithm bp(result_file);
    bp.play(game);
  }
  return 0;
}
