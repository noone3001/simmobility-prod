#include "Broker-util.hpp"
#include "entities/commsim/comm_support/AgentCommUtility.hpp"

namespace sim_mob
{
		AgentInfo::AgentInfo(Agent * a_, AgentCommUtilityBase * b_, bool v): agent(a_), comm(b_), valid(v), done(false){}
		AgentInfo::AgentInfo(){}

	//	function to insert into the container (only checkes the unqueness of agent)
	void AgentsList::insert(Agent * a, AgentCommUtilityBase * b, bool valid ){
		boost::unique_lock<boost::mutex> lock(mutex);
		Agents &aa = data.get<agent_tag>();
		aa.insert(AgentInfo(a,b,valid));
	};
	//	function to insert into the container (only checkes the unqueness of agent)
	int AgentsList::size( ) {
		boost::unique_lock<boost::mutex> lock(mutex);
		Agents &aa = data.get<agent_tag>();
//		Agents::iterator it = aa.begin(), it_end = aa.end();
//		for(; it!=it_end; it++) {
//			Print() << it->agent << std::endl;
//		}
		return aa.size();
	};

	bool AgentsList::empty()
	{
		boost::unique_lock<boost::mutex> lock(mutex);
		Agents &aa = data.get<agent_tag>();
		return aa.empty();

	}

	//	erases an elment from the container given the agent's reference
	void AgentsList::erase(Agent * agent) {
		boost::unique_lock<boost::mutex> lock(mutex);
		Agents &agents = data.get<agent_tag>();
		if(agents.find(agent) != agents.end())
		{
			Print() << "AgentsList::erase test:  agent[" << agent << "] found for deletion" << std::endl;
		}
		else{
			Print() << "AgentsList::erase test:  agent[" << agent << "] NOT found for deletion" << std::endl;
		}

		agents.erase(agent);

		Agents &agents1 = data.get<agent_tag>();
		if(agents1.find(agent) == agents1.end())
		{
			Print() << "AgentsList::erase test:  agent[" << agent << "] not found after deletion" << std::endl;
		}
		else{
			Print() << "AgentsList::erase test:  agent[" << agent << "] FOUND after deletion" << std::endl;
		}
	}

	//	erases an elment from the container given its communication equipment
	void AgentsList::erase(AgentCommUtilityBase * comm) {
		boost::unique_lock<boost::mutex> lock(mutex);
		AgentCommUtilities &comms = data.get<agent_util>();
		comms.erase(comm);
	}

	//erase all elements whose value of 'valid' is set to false
	void AgentsList::eraseInvalids() {
		boost::unique_lock<boost::mutex> lock(mutex);
		Print() << "AgentsList::eraseInvalids test" << std::endl;
		Valids & valids = data.get<agent_valid>();
		Valids::iterator it_begin, it_end;
		boost::tuples::tie(it_begin, it_end) = valids.equal_range(false);
		while(it_begin != it_end)
		{
			Print() << "AgentsList::eraseInvalids test-inloop" << std::endl;
			Valids::iterator it_erase = it_begin++;
			valids.erase(it_erase);
			Print() << "AgentsList::eraseInvalids test-inloop-after erase" << std::endl;
		}
	}

	//returns the container as if it is a vector of constant elements(to the best of my knowledge)
	AgentsList::type &AgentsList::getAgents(){
		return data.get<0>();
	};

	void AgentsList::for_each_agent(boost::function<void(sim_mob::Agent*)> Fn)
	{
		boost::unique_lock<boost::mutex> lock(mutex);
		AgentsList::type & agents = data.get<0>();
		for(AgentsList::iterator it= agents.begin(), it_end(agents.end()); it != it_end; it++ )
		{
			if(it->valid == false){
				Print() << "Warning, Working with an Invalid Agent" << std::endl;
			}
			Fn(it->agent);
		}
	}

	//	given an agent's reference, returns a const version of a single element from the container.
	const AgentInfo  &AgentsList::getAgentInfo(Agent * agent, bool &success){
		boost::unique_lock<boost::mutex> lock(mutex);
		success = false;
		AgentsList::Agents &agents = data.get<agent_tag>();
		AgentsList::Agents::iterator it;
		if((it = agents.find(agent)) != agents.end())
		{
			success = true;
		}
		return 	*it;
	};

	//	given a reference to an agent's comm equipment, returns a const version of a single element from the container.
	const AgentInfo  &AgentsList::getAgentInfo(AgentCommUtilityBase * comm, bool &success){
		boost::unique_lock<boost::mutex> lock(mutex);
		success = false;
		AgentsList::AgentCommUtilities &comms = data.get<agent_util>();
		AgentCommUtilities::iterator it;
		if((it = comms.find(comm)) != comms.end())
		{
			success = true;
		}
		return 	*it;
	};

	/*******************************************************************
	 * ********* VALID ************************************************
	 * ******************************************************************
	 */

	//validates an element give an agent
	//if found and if valid, return 1
	//if found and not valid, return 0
	//if not found return -1
	int AgentsList::Validate(Agent * agent) {
		boost::unique_lock<boost::mutex> lock(mutex);
		Agents &agents = data.get<agent_tag>();
		Agents::iterator it = agents.find(agent);
		if((it = agents.find(agent)) != agents.end())
		{
			if(it->valid)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		return -1;
	}

	//validates an element give an agent's comm equipment
	//if found and if valid, return 1
	//if found and not valid, return 0
	//if not found return -1
	int AgentsList::Validate(AgentCommUtilityBase * comm) {
		boost::unique_lock<boost::mutex> lock(mutex);
		AgentCommUtilities &comms = data.get<agent_util>();
		AgentCommUtilities::iterator it;
		if((it = comms.find(comm)) != comms.end())
		{
			if(it->valid)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		return -1;
	}

		AgentsList::change_valid::change_valid(bool new_value):new_value(new_value){}

	  void AgentsList::change_valid::operator()(AgentInfo& e)
	  {
	    e.valid=new_value;
	  }

	//	sets valid value of an element in the container, give an agent's comm equipment
	bool AgentsList::setValid(AgentCommUtilityBase * comm , bool value) {
		boost::unique_lock<boost::mutex> lock(mutex);
		bool success = false;
		AgentCommUtilities &comms = data.get<agent_util>();
		AgentCommUtilities::iterator it;
		if((it = comms.find(comm)) != comms.end())
		{
			comms.modify(it,change_valid(value));
		}
		return -1;
	}

	//	sets valid value of an element in the container, give an agent
	bool AgentsList::setValid(Agent * agent , bool value) {
		boost::unique_lock<boost::mutex> lock(mutex);
		bool success = false;
		Agents &agents = data.get<agent_tag>();
		Agents::iterator it;
		if((it = agents.find(agent)) != agents.end())
		{
			agents.modify(it,change_valid(value));
		}
		return -1;
	}
	/*******************************************************************
	 * ********* Done ************************************************
	 * ******************************************************************
	 */
//checks if an agent is done
	bool AgentsList::isDone(Agent * agent) {
		boost::unique_lock<boost::mutex> lock(mutex);
		Agents &agents = data.get<agent_tag>();
		Agents::iterator it = agents.find(agent);
		if((it = agents.find(agent)) != agents.end())
		{
			if(it->done)
			{
				return true;
		}
		}
		return false;
	}

		//checks if an agent is done
	bool AgentsList::isDone(AgentCommUtilityBase * comm) {
		boost::unique_lock<boost::mutex> lock(mutex);
		AgentCommUtilities &comms = data.get<agent_util>();
		AgentCommUtilities::iterator it;
		if((it = comms.find(comm)) != comms.end())
		{
			if(it->done)
			{
				return true;
		}
		}
		return false;
	}

	AgentsList::valid_range AgentsList::getNotDone()
	{
		boost::unique_lock<boost::mutex> lock(mutex);
		Valids &valids = data.get<agent_valid>();
		return valids.equal_range(false);;
	}



	AgentsList::change_done::change_done(bool new_value):new_value(new_value){}

	  void AgentsList::change_done::operator()(AgentInfo& e)
	  {
	    e.done=new_value;
	  }

	//	sets Done value of an element in the container, give an agent's comm equipment
	bool AgentsList::setDone(AgentCommUtilityBase * comm , bool value) {
		boost::unique_lock<boost::mutex> lock(mutex);
		bool success = false;
		AgentCommUtilities &comms = data.get<agent_util>();
		AgentCommUtilities::iterator it;
		if((it = comms.find(comm)) != comms.end())
		{
			comms.modify(it,change_done(value));
			return true;
		}
		return false;
	}

	//	sets Done value of an element in the container, give an agent
	bool AgentsList::setDone(Agent * agent , bool value) {
		boost::unique_lock<boost::mutex> lock(mutex);
		bool success = false;
		Agents &agents = data.get<agent_tag>();
		Agents::iterator it;
		if((it = agents.find(agent)) != agents.end())
		{
			agents.modify(it,change_done(value));
			return true;
		}
		return false;
	}

}//namespace sim_mob

