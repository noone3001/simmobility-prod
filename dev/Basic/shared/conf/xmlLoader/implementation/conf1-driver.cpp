// Not copyrighted - public domain.
//
// This sample parser implementation was generated by CodeSynthesis XSD,
// an XML Schema to C++ data binding compiler. You may use it in your
// programs without any restrictions.
//

#include "conf1-driver.hpp"

#include "conf1-pimpl.hpp"
#include <iostream>

bool sim_mob::xml::InitAndLoadConfigXML(const std::string& fileName, sim_mob::Config& resultConfig)
{
	  try
	  {
	    // Instantiate individual parsers.
	    //
	    ::sim_mob::conf::SimMobility_pimpl SimMobility_p;
	    ::sim_mob::conf::constructs_pimpl constructs_p;
	    ::sim_mob::conf::models_pimpl models_p;
	    ::sim_mob::conf::model_pimpl model_p;
	    ::xml_schema::string_pimpl string_p;
	    ::sim_mob::conf::workgroup_sizes_pimpl workgroup_sizes_p;
	    ::sim_mob::conf::workgroup_pimpl workgroup_p;
	    ::xml_schema::int_pimpl int_p;
	    ::sim_mob::conf::react_times_pimpl react_times_p;
	    ::sim_mob::conf::reaction_time_pimpl reaction_time_p;
	    ::sim_mob::conf::db_connections_pimpl db_connections_p;
	    ::sim_mob::conf::db_connection_pimpl db_connection_p;
	    ::sim_mob::conf::db_param_pimpl db_param_p;
	    ::sim_mob::conf::db_proc_groups_pimpl db_proc_groups_p;
	    ::sim_mob::conf::proc_map_pimpl proc_map_p;
	    ::sim_mob::conf::db_proc_mapping_pimpl db_proc_mapping_p;

	    // Connect the parsers together.
	    //
	    SimMobility_p.parsers (constructs_p);

	    constructs_p.parsers (models_p,
	                          workgroup_sizes_p,
	                          react_times_p,
	                          db_connections_p,
	                          db_proc_groups_p);

	    models_p.parsers (model_p,
	                      model_p,
	                      model_p,
	                      model_p);

	    model_p.parsers (string_p,
	                     string_p);

	    workgroup_sizes_p.parsers (workgroup_p,
	                               workgroup_p);

	    workgroup_p.parsers (int_p);

	    react_times_p.parsers (reaction_time_p,
	                           reaction_time_p);

	    reaction_time_p.parsers (string_p,
	                             int_p,
	                             int_p);

	    db_connections_p.parsers (db_connection_p);

	    db_connection_p.parsers (db_param_p,
	                             string_p,
	                             string_p);

	    db_param_p.parsers (string_p,
	                        string_p);

	    db_proc_groups_p.parsers (proc_map_p);

	    proc_map_p.parsers (db_proc_mapping_p,
	                        string_p,
	                        string_p);

	    db_proc_mapping_p.parsers (string_p,
	                               string_p);

	    // Parse the XML document.
	    //
	    ::xml_schema::document doc_p (
	      SimMobility_p,
	      "http://www.smart.mit.edu/conf",
	      "SimMobility");

	    SimMobility_p.pre ();
	    doc_p.parse (fileName);
	    SimMobility_p.post_SimMobility ();
	  }
  catch (const ::xml_schema::exception& e)
  {
    std::cerr << e << std::endl;
    return false;
  }
  catch (const std::ios_base::failure&)
  {
    std::cerr << fileName << ": error: io failure" << std::endl;
    return false;
  }

  return true;
}

