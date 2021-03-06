#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>

using namespace eosio;
using namespace std;

const uint64_t ONE_HOUR = 7200;

class [[eosio::contract]] blocktivity : public contract {
public:
    using contract::contract;

    /**
     * ## ACTION `push`
     *
     * Pushes hourly (7200 blocks) statistics of transaction & action counts.
     *
     * - Authority:  `get_self()`
     *
     * ### params
     *
     * - `{uint64_t} block_num` - block number start (rounded to the nearest 7200 interval)
     * - `{time_point_sec} timestamp` - block creation timestamp (UTC)
     * - `{uint64_t} transactions` - number of actions during 1 hour period
     * - `{uint64_t} actions` - number of transactions during 1 hour period
     * - `{uint64_t} cpu_usage_us` - number of cpu_usage_us during 1 hour period
     * - `{uint64_t} net_usage_words` - number of net_usage_words during 1 hour period
     *
     * ### example
     *
     * ```bash
     * cleos push action blocktivity push '[87458400, "2019-11-03T16:48:21", 299282, 281802]' -p blocktivity
     * ```
     */
    [[eosio::action]]
    void push( const uint64_t block_num, const eosio::time_point_sec timestamp, const uint64_t transactions, const uint64_t actions, const uint64_t cpu_usage_us, const uint64_t net_usage_words );

    using push_action = eosio::action_wrapper<"push"_n, &blocktivity::push>;

private:
    /**
     * ## TABLE `periods`
     *
     * - `{uint64_t} block_num` - start of block number
     * - `{time_point_sec} timestamp` - block creation timestamp (UTC)
     * - `{uint64_t} transactions` - number of actions during 1 hour period
     * - `{uint64_t} actions` - number of transactions during 1 hour period
     * - `{uint64_t} cpu_usage_us` - number of cpu_usage_us during 1 hour period
     * - `{uint64_t} net_usage_words` - number of net_usage_words during 1 hour period
     *
     * ### example
     *
     * ```json
     * {
     *   "block_num": 87458400,
     *   "timestamp": "2019-11-03T16:48:21",
     *   "transactions": 299282,
     *   "actions": 281802,
     *   "cpu_usage_us": 63476,
     *   "net_usage_words": 1772
     * }
     * ```
     */
    struct [[eosio::table("periods")]] periods_row {
        uint64_t                block_num;
        eosio::time_point_sec   timestamp;
        uint64_t                transactions;
        uint64_t                actions;
        uint64_t                cpu_usage_us = 0;
        uint64_t                net_usage_words = 0;

        uint64_t primary_key() const { return block_num * -1; }
    };

    /**
     * ## TABLE `sum`
     *
     * - `{uint64_t} hour` - hourly number of actions
     * - `{uint64_t} day` - daily number of actions
     * - `{uint64_t} week` - weekly number of actions
     * - `{time_point_sec} last_updated` - last updated (UTC)
     *
     * ### example
     *
     * ```json
     * {
     *   "hour": 875365,
     *   "day": 20773084,
     *   "week": 83237200,
     *   "last_updated": "2019-11-03T16:48:21"
     * }
     * ```
     */
    struct [[eosio::table("sum")]] sum_row {
        uint64_t                hour = 0;
        uint64_t                day = 0;
        uint64_t                week = 0;
        eosio::time_point_sec   last_updated;
    };

    /**
     * ## TABLE `average`
     *
     * - `{uint64_t} hour` - average hourly number of actions (7 day average)
     * - `{uint64_t} day` - average daily number of actions (7 day average)
     * - `{uint64_t} week` - weekly number of actions
     * - `{time_point_sec} last_updated` - last updated (UTC)
     *
     * ### example
     *
     * ```json
     * {
     *   "hour": 875365,
     *   "day": 20773084,
     *   "week": 83237200,
     *   "last_updated": "2019-11-03T16:48:21"
     * }
     * ```
     */
    struct [[eosio::table("average")]] average_row {
        uint64_t                hour = 0;
        uint64_t                day = 0;
        uint64_t                week = 0;
        eosio::time_point_sec   last_updated;
    };

    /**
     * ## TABLE `record`
     *
     * - `{uint64_t} hour` - highest hourly number of actions
     * - `{uint64_t} day` - highest daily number of actions
     * - `{uint64_t} week` - highest weekly number of actions
     * - `{time_point_sec} last_updated` - last updated (UTC)
     *
     * ### example
     *
     * ```json
     * {
     *   "hour": 875365,
     *   "day": 20773084,
     *   "week": 83237200,
     *   "last_updated": "2019-11-03T16:48:21"
     * }
     * ```
     */
    struct [[eosio::table("record")]] record_row {
        uint64_t                hour = 0;
        uint64_t                day = 0;
        uint64_t                week = 0;
        eosio::time_point_sec   last_updated;
    };

    // Tables
    typedef eosio::multi_index< "periods"_n, periods_row> periods_table;
    typedef eosio::singleton< "sum"_n, sum_row> sum_table;
    typedef eosio::singleton< "average"_n, average_row> average_table;
    typedef eosio::singleton< "record"_n, record_row> record_table;

    // private helpers
    void add_hour( const uint64_t block_num, const eosio::time_point_sec timestamp, const uint64_t transactions, const uint64_t actions, const uint64_t cpu_usage_us, const uint64_t net_usage_words );
    void calculate_periods( const uint64_t block_num );
};