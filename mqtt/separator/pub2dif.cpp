//separate data and send to different server.

#if !defined(_WIN32)
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <dirent.h>
	#include <unistd.h>
	#include <fstream>
#endif

#include <random>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include<fstream>
#include<sstream>
#include "mqtt/async_client.h"

using namespace std;
using namespace std::chrono;

const std::string DFLT_ADDRESS { "tcp://localhost:1883" };
const std::string CLIENT_ID { "paho-cpp-data-publish" };

const std::string DFLT_ADDRESS_2 { "mqtt.eclipseprojects.io" };
const std::string CLIENT_ID_2 { "paho-cpp-data-publish_2" };


const string TOPIC { "data/rand" };
const int	 QOS = 1;

const auto PERIOD = seconds(5);

const int MAX_BUFFERED_MSGS = 120;	// 120 * 5sec => 10min off-line buffering

const string PERSIST_DIR { "data-persist" };

/////////////////////////////////////////////////////////////////////////////

// At some point, when the library gets updated to C++17, we can use
// std::filesystem to make a portable version of this.

#if !defined(_WIN32)

// Example of user-based file persistence with a simple XOR encoding scheme.
//
// Similar to the built-in file persistence, this just creates a
// subdirectory for the persistence data, then places each key into a
// separate file using the key as the file name.
//
// With user-defined persistence, you can transform the data in any way you
// like, such as with encryption/decryption, and you can store the data any
// place you want, such as here with disk files, or use a local DB like
// SQLite or a local key/value store like Redis.
class encoded_file_persistence : virtual public mqtt::iclient_persistence
{
	// The name of the store
	// Used as the directory name
	string name_;

	// A key for encoding the data
	string encodeKey_;

	// Simple, in-place XOR encoding and decoding
	void encode(string& s) const {
		size_t n = encodeKey_.size();
		if (n == 0 || s.empty()) return;

		for (size_t i=0; i<s.size(); ++i)
			s[i] ^= encodeKey_[i%n];
	}

	// Gets the persistence file name for the supplied key.
	string path_name(const string& key) const { return name_ + "/" + key; }

public:
	// Create the persistence object with the specified encoding key
	encoded_file_persistence(const string& encodeKey)
			: encodeKey_(encodeKey) {}

	// "Open" the persistence store.
	// Create a directory for persistence files, using the client ID and
	// serverURI to make a unique directory name. Note that neither can be
	// empty. In particular, the app can't use an empty `clientID` if it
	// wants to use persistence. (This isn't an absolute rule for your own
	// persistence, but you do need a way to keep data from different apps
	// separate).
	void open(const string& clientId, const string& serverURI) override {
		if (clientId.empty() || serverURI.empty())
			throw mqtt::persistence_exception();

		name_ = serverURI + "-" + clientId;
		std::replace(name_.begin(), name_.end(), ':', '-');

		mkdir(name_.c_str(), S_IRWXU | S_IRWXG);
	}

	// Close the persistent store that was previously opened.
	// Remove the persistence directory, if it's empty.
	void close() override {
		rmdir(name_.c_str());
	}

	// Clears persistence, so that it no longer contains any persisted data.
	// Just remove all the files from the persistence directory.
	void clear() override {
		DIR* dir = opendir(name_.c_str());
		if (!dir) return;

		dirent *next;
		while ((next = readdir(dir)) != nullptr) {
			auto fname = string(next->d_name);
			if (fname == "." || fname == "..") continue;
			string path = name_ + "/" + fname;
			remove(path.c_str());
		}
		closedir(dir);
	}

	// Returns whether or not data is persisted using the specified key.
	// We just look for a file in the store directory with the same name as
	// the key.
	bool contains_key(const string& key) override {
		DIR* dir = opendir(name_.c_str());
		if (!dir) return false;

		dirent *next;
		while ((next = readdir(dir)) != nullptr) {
			if (string(next->d_name) == key) {
				closedir(dir);
				return true;
			}
		}
		closedir(dir);
		return false;
	}

	// Returns the keys in this persistent data store.
	// We just make a collection of the file names in the store directory.
	mqtt::string_collection keys() const override {
		mqtt::string_collection ks;
		DIR* dir = opendir(name_.c_str());
		if (!dir) return ks;

		dirent *next;
		while ((next = readdir(dir)) != nullptr) {
			auto fname = string(next->d_name);
			if (fname == "." || fname == "..") continue;
			ks.push_back(fname);
		}

		closedir(dir);
		return ks;
	}

	// Puts the specified data into the persistent store.
	// We just encode the data and write it to a file using the key as the
	// name of the file. The multiple buffers given here need to be written
	// in order - and a scatter/gather like writev() would be fine. But...
	// the data will be read back as a single buffer, so here we first
	// concat a string so that the encoding key lines up with the data the
	// same way it will on the read-back.
	void put(const string& key, const std::vector<mqtt::string_view>& bufs) override {
		auto path = path_name(key);

		ofstream os(path, ios_base::binary);
		if (!os)
			throw mqtt::persistence_exception();

		string s;
		for (const auto& b : bufs)
			s.append(b.data(), b.size());

		encode(s);
		os.write(s.data(), s.size());
	}

	// Gets the specified data out of the persistent store.
	// We look for a file with the name of the key, read the contents,
	// decode, and return it.
	string get(const string& key) const override {
		auto path = path_name(key);

		ifstream is(path, ios_base::ate|ios_base::binary);
		if (!is)
			throw mqtt::persistence_exception();

		// Read the whole file into a string
		streamsize sz = is.tellg();
		if (sz == 0) return string();

		is.seekg(0);
		string s(sz, '\0');
		is.read(&s[0], sz);
		if (is.gcount() < sz)
			s.resize(is.gcount());

		encode(s);
		return s;
	}

	// Remove the data for the specified key.
	// Just remove the file with the same name as the key, if found.
	void remove(const string &key) override {
		auto path = path_name(key);
		::remove(path.c_str());
	}
};
#endif


bool word_checker(string s){
    int h=0;
    for(int i=0;i<s.length();i++){
        if(s[i]<'0'||s[i]>'9'){
            if(s[i]!='.'){
                h=2;
                break;
            }
            else h++;
        }
    }
    if(h<2)return 0;
    else return 1;
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	string address = (argc > 1) ? string(argv[1]) : DFLT_ADDRESS;

	#if defined(_WIN32)
		mqtt::async_client cli(address, CLIENT_ID, MAX_BUFFERED_MSGS);
	#else
		encoded_file_persistence persist("elephant");
		mqtt::async_client cli(address, CLIENT_ID, MAX_BUFFERED_MSGS, &persist);
	#endif

    mqtt::async_client cli_2(DFLT_ADDRESS_2, CLIENT_ID_2, MAX_BUFFERED_MSGS, &persist);

	auto connOpts = mqtt::connect_options_builder()
		.keep_alive_interval(MAX_BUFFERED_MSGS * PERIOD)
		.clean_session(true)
		.automatic_reconnect(true)
		.finalize();

	// Create a topic object. This is a conventience since we will
	// repeatedly publish messages with the same parameters.
	mqtt::topic top(cli, TOPIC, QOS, true);
    mqtt::topic top_2(cli_2, TOPIC, QOS, true);

	// Random number generator [0 - 100]
	random_device rnd;
    mt19937 gen(rnd());
    uniform_int_distribution<> dis(0, 100);

	try {
		// Connect to the MQTT broker
		cout << "Connecting to server '" << address << "'..." << flush;
		cli.connect(connOpts)->wait();
        cli_2.connect(connOpts)->wait();
		cout << "OK\n" << endl;

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
			        
			        cout << word << endl;

			        // Publish to the topic
			        if(word_checker(word)==1)top.publish(std::move(word));
                    else top_2.publish(std::move(word));
			        tm += PERIOD;

                }
            }
        }

		// Disconnect
		cout << "\nDisconnecting..." << flush;
		cli.disconnect()->wait();
		cout << "OK" << endl;
	}
	catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
		return 1;
	}

 	return 0;
}

