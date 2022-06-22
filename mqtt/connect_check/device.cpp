//first give status then send.

#include <iostream>
#include <cstdlib>
#include <string>
#include <ctime>
#include <cstdlib>
#include<fstream>
#include<sstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include "mqtt/async_client.h"

using namespace std;
using namespace std::chrono;

const string DFLT_SERVER_ADDRESS	{ "tcp://localhost:1883" };
const string CLIENT_ID				{ "device_publisher" };
const string PERSIST_DIR			{ "./persist" };

const string TOPIC1 { "status/device" };
const string TOPIC2 { "data" };

const char* PAYLOAD1 = "true";
const char* PAYLOAD0 = "0";

const char* LWT_PAYLOAD = "0";
const auto PERIOD = seconds(5);

const int  QOS = 1;

const auto TIMEOUT = std::chrono::seconds(10);

/////////////////////////////////////////////////////////////////////////////

/**
 * A callback class for use with the main MQTT client.
 */
class callback : public virtual mqtt::callback
{
public:
	void connection_lost(const string& cause) override {
		cout << "\nConnection lost" << endl;
		if (!cause.empty())
			cout << "\tcause: " << cause << endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr tok) override {
		cout << "\tDelivery complete for token: "
			<< (tok ? tok->get_message_id() : -1) << endl;
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A base action listener.
 */
class action_listener : public virtual mqtt::iaction_listener
{
protected:
	void on_failure(const mqtt::token& tok) override {
		cout << "\tListener failure for token: "
			<< tok.get_message_id() << endl;
	}

	void on_success(const mqtt::token& tok) override {
		cout << "\tListener success for token: "
			<< tok.get_message_id() << endl;
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A derived action listener for publish events.
 */
class delivery_action_listener : public action_listener
{
	atomic<bool> done_;

	void on_failure(const mqtt::token& tok) override {
		action_listener::on_failure(tok);
		done_ = true;
	}

	void on_success(const mqtt::token& tok) override {
		action_listener::on_success(tok);
		done_ = true;
	}

public:
	delivery_action_listener() : done_(false) {}
	bool is_done() const { return done_; }
};

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	// A client that just publishes normally doesn't need a persistent
	// session or Client ID unless it's using persistence, then the local
	// library requires an ID to identify the persistence files.

	string	address  = (argc > 1) ? string(argv[1]) : DFLT_SERVER_ADDRESS,
			clientID = (argc > 2) ? string(argv[2]) : CLIENT_ID;

	cout << "Initializing for server '" << address << "'..." << endl;
	mqtt::async_client client(address, clientID, PERSIST_DIR);

	callback cb;
	client.set_callback(cb);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session()
		.will(mqtt::message(TOPIC1, LWT_PAYLOAD, QOS))
		.finalize();

	cout << "  ...OK" << endl;

	try {
		cout << "\nConnecting..." << endl;
		mqtt::token_ptr conntok = client.connect(connOpts);
		cout << "Waiting for the connection..." << endl;
		conntok->wait();
		cout << "  ...OK" << endl;

		// Published status of device

		cout << "\nSending message..." << endl;
		mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC1, PAYLOAD1);
		pubmsg->set_qos(QOS);
		client.publish(pubmsg)->wait_for(TIMEOUT);
		cout << "  ...OK" << endl;



        // data publish
        mqtt::topic top(client, TOPIC2, QOS, true);
        char tmbuf[32];
		unsigned nsample = 0;

		// The time at which to reads the next sample, starting now
	

        string fname="data.csv";
        string line, word;
		auto tm = steady_clock::now();
 
        fstream file (fname, ios::in);
        if(file.is_open()){
            while(getline(file, line)){stringstream str(line);
                while(getline(str, word, '\n')){
			        // Pace the samples to the desired rate
			        this_thread::sleep_until(tm);

			        // Get a timestamp and format as a string
			        time_t t = system_clock::to_time_t(system_clock::now());
			        strftime(tmbuf, sizeof(tmbuf), "%F %T", localtime(&t));

			        // Simulate reading some data
			        //int x = dis(gen);

			        // Create the payload as a text CSV string
			        string payload = word;
			        cout << payload << endl;

			        // Publish to the topic
			        top.publish(std::move(payload));

			        tm += PERIOD;

                }
            }
        }

		// Disconnect
        mqtt::message_ptr pubmsg1 = mqtt::make_message(TOPIC1, PAYLOAD0);
		pubmsg->set_qos(QOS);
		client.publish(pubmsg)->wait_for(TIMEOUT);
		cout << "\nDisconnecting..." << endl;
		client.disconnect()->wait();
		cout << "  ...OK" << endl;
	}
	catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
		return 1;
	}

 	return 0;
}

