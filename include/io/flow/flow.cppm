export module io_base_flow;

import std;
import shared;
import io_base_leverage;

export import io_flow_sender;
export import io_flow_receiver;
export import io_flow_socket;


export namespace io::base::flow {

template <::shared::HandlerController Controller, typename... Ts>
class Flow {
  public:
    Flow(leverage::Leverager<leverage::Context> &lev, Controller ctrl) : m_leverager(lev), m_controller(ctrl) {}

    template <typename T, typename... Args>
        requires ::shared::FlowBase<T, Controller, leverage::Leverager<leverage::Context>>
    auto add(::shared::ReadCallback shared, Args... args) {
        auto recipe = [=](auto &lev, auto &ctrl) { return T{shared, lev, ctrl, args...}; };
        return Flow<Controller, Ts..., decltype(recipe)>{m_leverager, m_controller,
                                                         std::tuple_cat(m_recipes, std::make_tuple(recipe))};
    }

    // Build: Triggers instantiation
    auto build() {
        return std::apply([&](auto &...recipes) { return std::make_tuple(recipes(m_leverager, m_controller)...); },
                          m_recipes);
    }

  private:
    // Private constructor for internal builder transitions
    Flow(leverage::Leverager<leverage::Context> &lev, Controller ctrl, auto recipes)
        : m_leverager(lev), m_controller(ctrl), m_recipes(recipes) {}

    leverage::Leverager<leverage::Context> &m_leverager;
    Controller m_controller;
    std::tuple<Ts...> m_recipes = std::make_tuple();
};

} // namespace io::base::flow
