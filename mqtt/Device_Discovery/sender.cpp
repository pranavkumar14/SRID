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
string TOPIC = "data/subs";
const char* PAYLOAD1 = "true";
const char* PAYLOAD0 = "0";

const char* LWT_PAYLOAD = "0";
const auto PERIOD = seconds(5);

const int  QOS = 1;

const auto TIMEOUT = std::chrono::seconds(10);
bool flag =0;
const int	N_RETRY_ATTEMPTS = 5;
/////////////////////////////////////////////////////////////////////////////


class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
		std::cout << name_ << " failure";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		std::cout << std::endl;
	}

	void on_success(const mqtt::token& tok) override {
		std::cout << name_ << " success";
		if (tok.get_message_id() != 0)
			std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
		auto top = tok.get_topics();
		if (top && !top->empty())
			std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
		std::cout << std::endl;
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback,
					public virtual mqtt::iaction_listener

{
	// Counter for the number of connection retries
	int nretry_;
	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		try {
			cli_.connect(connOpts_, nullptr, *this);
		}
		catch (const mqtt::exception& exc) {
			std::cerr << "Error: " << exc.what() << std::endl;
			exit(1);
		}
	}

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override {
		std::cout << "Connection attempt failed" << std::endl;
		if (++nretry_ > N_RETRY_ATTEMPTS)
			exit(1);
		reconnect();
	}

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override {}

	// (Re)connection success
	void connected(const std::string& cause) override {
		std::cout << "\nConnection success" << std::endl;
		std::cout << "\nSubscribing to topic '" << TOPIC << "'\n"
			<< "\tfor client " << CLIENT_ID
			<< " using QoS" << QOS << "\n"
			<< "\nPress Q<Enter> to quit\n" << std::endl;

		cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;

		std::cout << "Reconnecting..." << std::endl;
		nretry_ = 0;
		reconnect();
	}

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override {
		std::cout << "Message arrived" << std::endl;
		std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
		std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
        if(msg->to_string() == "true"){
            flag = 1;
           
        }
		 std::cout<<"Flag: "<<flag<<std::endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
				: nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

/////////////////////////////////////////////////////////////////////////////




int main(int argc, char* argv[])
{


	string	address = DFLT_SERVER_ADDRESS,
			clientID = CLIENT_ID;

	cout << "Initializing for server '" << address << "'..." << endl;
	mqtt::async_client client(address, clientID, PERSIST_DIR);

	mqtt::callback cb;
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

	//subscribe to status of status/subs

	mqtt::async_client cli(address, CLIENT_ID);

	mqtt::connect_options connOpts;
	connOpts.set_clean_session(false);

	// Install the callback(s) before connecting.
	callback cb(cli, connOpts);
	cli.set_callback(cb);

	// Start the connection.
	// When completed, the callback will subscribe to topic.

	try {
		std::cout << "Connecting to the MQTT server..." << std::flush;
		cli.connect(connOpts, nullptr, cb);
	}
	catch (const mqtt::exception& exc) {
		std::cerr << "\nERROR: Unable to connect to MQTT server: '"
			<< address << "'" << exc << std::endl;
		return 1;
	}
	while(1){
		if(flag == 1)break;
	}
		try {
		std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
		cli.disconnect()->wait();
		std::cout << "OK" << std::endl;
	}
	catch (const mqtt::exception& exc) {
		std::cerr << exc << std::endl;
		return 1;
	}

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
