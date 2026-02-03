#include <charconv>
#include <metrics.hpp>

namespace metric_collector::aggregation
{
template <typename F>
concept MetricCallback =
    requires(F&& func, std::string_view name, MetricType type, uint64_t value) {
        { func(name, type, value) } -> std::same_as<void>;
    };

class Parser
{
  public:
    template <MetricCallback Callback>
    static void parse_packet(std::string_view packet, Callback&& cb)
    {
        size_t pos = 0;

        while (pos < packet.size())
        {
            size_t end = packet.find('\n', pos);
            if (end == std::string_view::npos)
            {
                end = packet.size();
            }

            std::string_view line = packet.substr(pos, end - pos);
            if (!line.empty())
            {
                parse_metric(line, cb);
            }

            pos = end + 1;
        }
    }

    template <MetricCallback Callback>
    static void parse_metric(std::string_view line, Callback&& cb)
    {
        auto colon = line.find(':');
        if (colon == std::string_view::npos)
        {
            return;
        }

        auto pipe = line.find('|', colon + 1);
        if (pipe == std::string_view::npos)
        {
            return;
        }

        std::string_view name     = line.substr(0, colon);
        std::string_view value_sv = line.substr(colon + 1, pipe - colon - 1);
        std::string_view type_sv  = line.substr(pipe + 1);

        int64_t value{};
        auto    res = std::from_chars(value_sv.data(), value_sv.data() + value_sv.size(), value);
        if (res.ec != std::errc{})
        {
            return;
        }

        cb(name, type_sv, value);
    }
};
} // namespace metric_collector::aggregation