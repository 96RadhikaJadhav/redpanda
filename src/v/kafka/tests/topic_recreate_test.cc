#include "kafka/errors.h"
#include "kafka/requests/batch_consumer.h"
#include "kafka/requests/delete_topics_request.h"
#include "kafka/requests/metadata_request.h"
#include "kafka/requests/produce_request.h"
#include "kafka/requests/topics/types.h"
#include "kafka/types.h"
#include "model/fundamental.h"
#include "model/record_batch_reader.h"
#include "model/timeout_clock.h"
#include "redpanda/application.h"
#include "redpanda/tests/fixture.h"
#include "storage/tests/utils/random_batch.h"
#include "test_utils/async.h"

#include <seastar/core/do_with.hh>
#include <seastar/core/sleep.hh>

#include <absl/container/flat_hash_map.h>
#include <boost/test/tools/old/interface.hpp>

#include <algorithm>
#include <chrono>
#include <iterator>
#include <optional>
#include <vector>

using namespace std::chrono_literals; // NOLINT

class recreate_test_fixture : public redpanda_thread_fixture {
public:
    void create_topic(ss::sstring tp, uint32_t partitions, uint16_t rf) {
        kafka::new_topic_configuration topic;

        topic.topic = model::topic(tp);
        topic.partition_count = partitions;
        topic.replication_factor = rf;

        std::vector<kafka::new_topic_configuration> topics;
        topics.push_back(std::move(topic));
        auto req = kafka::create_topics_request{
          .topics = std::move(topics),
          .timeout = 10s,
          .validate_only = false,
        };

        auto client = make_kafka_client().get0();
        client.connect().get0();
        auto resp
          = client.dispatch(std::move(req), kafka::api_version(2)).get0();
    }
    kafka::delete_topics_request make_delete_topics_request(
      std::vector<model::topic> topics, std::chrono::milliseconds timeout) {
        kafka::delete_topics_request req;
        req.data.topic_names = std::move(topics);
        req.data.timeout_ms = timeout;
        return req;
    }

    kafka::delete_topics_response
    delete_topics(std::vector<model::topic> topics) {
        return send_delete_topics_request(
          make_delete_topics_request(std::move(topics), 5s));
    }

    kafka::delete_topics_response
    send_delete_topics_request(kafka::delete_topics_request req) {
        auto client = make_kafka_client().get0();
        client.connect().get0();

        return client.dispatch(std::move(req), kafka::api_version(2)).get0();
    }

    kafka::metadata_response get_topic_metadata(const model::topic& tp) {
        auto client = make_kafka_client().get0();
        client.connect().get0();
        std::vector<model::topic> topics;
        topics.push_back(tp);
        kafka::metadata_request md_req{
          .topics = topics,
          .allow_auto_topic_creation = false,
          .list_all_topics = false};
        return client.dispatch(md_req).get0();
    }
};

FIXTURE_TEST(test_topic_recreation, recreate_test_fixture) {
    wait_for_controller_leadership().get();
    model::topic test_tp{"topic-1"};
    create_topic(test_tp(), 6, 1);
    delete_topics({test_tp});
    create_topic(test_tp(), 6, 1);

    auto md = get_topic_metadata(test_tp);
    BOOST_REQUIRE_EQUAL(md.topics.size(), 1);
    BOOST_REQUIRE_EQUAL(md.topics.begin()->partitions.size(), 6);

    for (auto& p : md.topics.begin()->partitions) {
        BOOST_REQUIRE_EQUAL(p.leader, model::node_id{1});
    }
}