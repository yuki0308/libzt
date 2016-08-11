/*
 * ZeroTier One - Network Virtualization Everywhere
 * Copyright (C) 2011-2016  ZeroTier, Inc.  https://www.zerotier.com/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ZT_ONESERVICE_HPP
#define ZT_ONESERVICE_HPP

#include <string>
#include <map>

#include "../node/Node.hpp"

// Include the right tap device driver for this platform -- add new platforms here
#ifdef SDK
	// In network containers builds, use the virtual netcon endpoint instead of a tun/tap port driver
	#include "../src/SDK_EthernetTap.hpp"
	namespace ZeroTier { typedef NetconEthernetTap EthernetTap; }
#endif // not ZT_SDK so pick a tap driver

namespace ZeroTier {

/**
 * Local service for ZeroTier One as system VPN/NFV provider
 *
 * If built with ZT_ENABLE_NETWORK_CONTROLLER defined, this includes and
 * runs controller/SqliteNetworkController with a database called
 * controller.db in the specified home directory.
 *
 * If built with ZT_AUTO_UPDATE, an official ZeroTier update URL is
 * periodically checked and updates are automatically downloaded, verified
 * against a built-in list of update signing keys, and installed. This is
 * only supported for certain platforms.
 *
 * If built with ZT_ENABLE_CLUSTER, a 'cluster' file is checked and if
 * present is read to determine the identity of other cluster members.
 */
class OneService
{
public:
	/**
	 * Returned by node main if/when it terminates
	 */
	enum ReasonForTermination
	{
		/**
		 * Instance is still running
		 */
		ONE_STILL_RUNNING = 0,

		/**
		 * Normal shutdown
		 */
		ONE_NORMAL_TERMINATION = 1,

		/**
		 * A serious unrecoverable error has occurred
		 */
		ONE_UNRECOVERABLE_ERROR = 2,

		/**
		 * Your identity has collided with another
		 */
		ONE_IDENTITY_COLLISION = 3
	};

	/**
	 * @return Platform default home path or empty string if this platform doesn't have one
	 */
	static std::string platformDefaultHomePath();

	/**
	 * @return Auto-update URL or empty string if auto-updates unsupported or not enabled
	 */
	static std::string autoUpdateUrl();

	/**
	 * Create a new instance of the service
	 *
	 * Once created, you must call the run() method to actually start
	 * processing.
	 *
	 * The port is saved to a file in the home path called zerotier-one.port,
	 * which is used by the CLI and can be used to see which port was chosen if
	 * 0 (random port) is picked.
	 *
	 * @param hp Home path
	 * @param port TCP and UDP port for packets and HTTP control (if 0, pick random port)
	 */
	static OneService *newInstance(
		const char *hp,
		unsigned int port);

	virtual ~OneService();

	/**
	 * Execute the service main I/O loop until terminated
	 *
	 * The terminate() method may be called from a signal handler or another
	 * thread to terminate execution. Otherwise this will not return unless
	 * another condition terminates execution such as a fatal error.
	 */
	virtual ReasonForTermination run() = 0;

	/**
	 * @return Reason for terminating or ONE_STILL_RUNNING if running
	 */
	virtual ReasonForTermination reasonForTermination() const = 0;

	/**
	 * @return Fatal error message or empty string if none
	 */
	virtual std::string fatalErrorMessage() const = 0;

	/**
	 * @return System device name corresponding with a given ZeroTier network ID or empty string if not opened yet or network ID not found
	 */
	virtual std::string portDeviceName(uint64_t nwid) const = 0;

	/**
	 * @return True if TCP fallback is currently active
	 */
	virtual bool tcpFallbackActive() const = 0;

	/**
	 * Terminate background service (can be called from other threads)
	 */
	virtual void terminate() = 0;

#ifdef SDK
	/**
	 * Joins a network
	 */
	virtual void join(const char *hp) = 0;
    
    /**
     * Leaves a network
     */
    virtual void leave(const char *hp) = 0;

    /**
     * Returns the homePath given by the client application
     * - Used for SDK mode
     */
    virtual std::string givenHomePath() = 0;

	/*
	 *
	 */
	virtual std::map< uint64_t,EthernetTap * > getTaps() = 0;

	/*
	 *
	 */
	virtual Node * getNode() = 0;

	/**
	 * @return True if service is still running
	 */
	inline bool isRunning() const { return (this->reasonForTermination() == ONE_STILL_RUNNING); }
#endif

protected:
	OneService() {}

private:
	OneService(const OneService &one) {}
	inline OneService &operator=(const OneService &one) { return *this; }
};

} // namespace ZeroTier

#endif
