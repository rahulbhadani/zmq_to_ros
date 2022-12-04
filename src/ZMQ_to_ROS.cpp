#include <zmq.hpp>
#include "ros/ros.h"
#include "std_msgs/Float64.h"

using namespace std;

// ZmqToROS1 is a class that converts ZMQ messages to ROS 1 messages and publishes them on a ROS 1 topic
class ZmqToROS1
{
	public:
		// The constructor initializes a ZMQ subscriber socket and a ROS 1 publisher
		// The topic parameter specifies the topic on which the ROS 1 publisher will publish messages
		ZmqToROS1(const std::string& topic): topic_(topic)
		{
			// Initialize a ZMQ context and create a ZMQ socket
			context = zmq_ctx_new();
			subscriber =  zmq_socket (context, ZMQ_SUB);

			// Connect the ZMQ socket to the ZMQ server running on localhost on port 4242
			zmq_connect (subscriber, "tcp://localhost:4242");

			// Set the socket to only receive messages that start with the letter "B"
			zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, "B", 1);

			// Create a ROS 1 publisher that publishes messages of type std_msgs::Float64 on the specified topic
			pub_ = nh_.advertise<std_msgs::Float64>(topic, 1);
		}

		// The run() method receives messages from the ZMQ socket, converts them to ROS 1 messages, and publishes them on the ROS 1 topic
		void run()
		{
			// Receive messages from the ZMQ socket in an infinite loop
			while (true)
			{
				// Create a ZMQ message
				zmq_msg_t msg;
				zmq_msg_init(&msg);

				// Receive a message on the ZMQ socket
				int rc = zmq_msg_recv(&msg, this->subscriber, 0);

				// If the context has been terminated, exit the loop
				if (rc < 0)
					continue;

				// Convert the ZMQ message to a ROS 1 message
				std_msgs::Float64 ros_msg;

				// Get the size of the ZMQ message
				size_t size = zmq_msg_size(&msg);

				// Allocate memory for the message data, copy the message data from the ZMQ message, and null-terminate the string
				char *string = (char*)malloc(size + 1);
				memcpy(string, zmq_msg_data(&msg), size);
				zmq_msg_close(&msg);
				string[size] = 0;

				// If the received message is the topic of the ROS 1 publisher, skip it
				if(string == this->topic_)
					continue;
					
				double message_value = stod(string);

				ros_msg.data = message_value;

				// Publish the message on the ROS 1 topic
				pub_.publish(ros_msg);
			}
		}

	private:
		void *context;
		void *subscriber;
		const std::string& topic_;
		ros::NodeHandle nh_;
		ros::Publisher pub_;
};

int main(int argc, char** argv)
{
	// Initialize ROS 1
	ros::init(argc, argv, "zmq_to_ros1");

	// Create the ZmqToROS1 class and run it
	ZmqToROS1 converter("B");

	cout <<"socket creation"<<endl;
	converter.run();

	return 0;
}
