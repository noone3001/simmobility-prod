//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "geo10-driver.hpp"

#include <iostream>

#include "geo10-pimpl.hpp"
#include "entities/misc/TripChain.hpp"


namespace {

/**
 * Because of the massive amount of initialization required by XSD, we cheat a bit and use this function to provide all of our
 *   higher-level functionality. As of now, the function does the following:
 *     1) If "resultNetwork" is non-null and "resultTripChains" is non-null, attempt to load an entire "SimMobility" spec.
 *     2) If "resultNetwork" is null, and "resultTripChains" is non-null, attempt to load just the "TripChains" section.
 */
bool init_and_load_internal(const std::string& fileName, const std::string& rootNode, sim_mob::RoadNetwork* resultNetwork, std::map<std::string, std::vector<sim_mob::TripChainItem*> >* resultTripChains)
{
	//Attempt to load
	try {
		//Our bookkeeper assists various classes with optimizations, and is shared between them.
		::sim_mob::xml::helper::Bookkeeping book;
		::sim_mob::xml::helper::SignalHelper signal;

		//Complex (usually optimized) parsers require external information.
		::sim_mob::xml::RoadNetwork_t_pimpl RoadNetwork_t_p(resultNetwork);

		//Simple, optimized with the book-keeper only.
		::sim_mob::xml::Nodes_pimpl Nodes_p(book);
		::sim_mob::xml::Activity_t_pimpl Activity_t_p(book);
		::sim_mob::xml::GeoSpatial_t_pimpl GeoSpatial_t_p(book);
		::sim_mob::xml::link_t_pimpl link_t_p(book);
		::sim_mob::xml::segment_t_pimpl segment_t_p(book);
		::sim_mob::xml::Trip_t_pimpl Trip_t_p(book);
		::sim_mob::xml::SubTrip_t_pimpl SubTrip_t_p(book);
		::sim_mob::xml::Lanes_pimpl Lanes_p(book);
	//	::sim_mob::xml::Segments_pimpl Segments_p(book);
		::sim_mob::xml::Links_pimpl Links_p(book);
		::sim_mob::xml::linkAndCrossing_t_pimpl linkAndCrossing_t_p(book);
		::sim_mob::xml::UniNode_t_pimpl UniNode_t_p(book);
		::sim_mob::xml::intersection_t_pimpl intersection_t_p(book);
		::sim_mob::xml::fwdBckSegments_t_pimpl fwdBckSegments_t_p(book);

		//Also simple, but for TripChains.
		::sim_mob::xml::TripChains_t_pimpl TripChains_t_p(resultTripChains);

		//Trivial parsers
	    ::sim_mob::xml::SimMobility_t_pimpl SimMobility_t_p;
	    ::sim_mob::xml::UniNodes_pimpl UniNodes_p;
	    ::xml_schema::unsigned_int_pimpl unsigned_int_p;
	    ::sim_mob::xml::Point2D_t_pimpl Point2D_t_p;
	    ::sim_mob::xml::coordinate_map_t_pimpl coordinate_map_t_p;
	    ::sim_mob::xml::roadrunner_regions_t_pimpl road_runner_regions_t_p;
	    ::sim_mob::xml::roadrunner_region_t_pimpl roadrunner_region_t_p;
	    ::sim_mob::xml::roadrunner_shape_t_pimpl roadrunner_shape_t_p;
	    ::sim_mob::xml::roadrunner_vertex_t_pimpl roadrunner_vertex_t_p;
	    ::sim_mob::xml::utm_projection_t_pimpl utm_projection_t_p;
	    ::sim_mob::xml::linear_scale_t_pimpl linear_scale_t_p;
	    ::sim_mob::xml::scale_source_t_pimpl scale_source_t_p;
	    ::sim_mob::xml::scale_destination_t_pimpl scale_destination_t_p;
	    ::xml_schema::string_pimpl string_p;
	    ::sim_mob::xml::temp_Segmetair_t_pimpl temp_Segmetair_t_p;
	    ::xml_schema::unsigned_long_pimpl unsigned_long_p;
	    ::sim_mob::xml::connectors_t_pimpl connectors_t_p;
	    ::sim_mob::xml::connector_t_pimpl connector_t_p;
	    ::sim_mob::xml::Intersections_pimpl Intersections_p;
	    ::sim_mob::xml::RoadSegmentsAt_t_pimpl RoadSegmentsAt_t_p;
	    ::sim_mob::xml::Multi_Connectors_t_pimpl Multi_Connectors_t_p;
	    ::sim_mob::xml::Multi_Connector_t_pimpl Multi_Connector_t_p;
	    ::sim_mob::xml::ChunkLengths_t_pimpl ChunkLengths_t_p;
	    ::sim_mob::xml::ChunkLength_t_pimpl ChunkLength_t_p;
	    ::xml_schema::unsigned_short_pimpl unsigned_short_p;
	    ::sim_mob::xml::offsets_t_pimpl offsets_t_p;
	    ::sim_mob::xml::offset_t_pimpl offset_t_p;
	    ::sim_mob::xml::separators_t_pimpl separators_t_p;
	    ::sim_mob::xml::separator_t_pimpl separator_t_p;
	    ::xml_schema::boolean_pimpl boolean_p;
	    ::sim_mob::xml::LanesVector_t_pimpl LanesVector_t_p;
	    ::sim_mob::xml::DomainIslands_t_pimpl DomainIslands_t_p;
	    ::sim_mob::xml::DomainIsland_t_pimpl DomainIsland_t_p;
	    ::sim_mob::xml::roundabouts_pimpl roundabouts_p;
	    ::sim_mob::xml::roundabout_t_pimpl roundabout_t_p;
	    ::xml_schema::float_pimpl float_p;
	    ::xml_schema::int_pimpl int_p;
	    ::sim_mob::xml::EntranceAngles_t_pimpl EntranceAngles_t_p;
	    ::sim_mob::xml::EntranceAngle_t_pimpl EntranceAngle_t_p;
	    ::xml_schema::short_pimpl short_p;
	    ::sim_mob::xml::PolyLine_t_pimpl PolyLine_t_p;
	    ::sim_mob::xml::PolyPoint_t_pimpl PolyPoint_t_p;
	    ::sim_mob::xml::laneEdgePolylines_cached_t_pimpl laneEdgePolylines_cached_t_p;
	    ::sim_mob::xml::laneEdgePolyline_cached_t_pimpl laneEdgePolyline_cached_t_p;
	    ::sim_mob::xml::lane_t_pimpl lane_t_p;
	    ::sim_mob::xml::RoadItems_t_pimpl RoadItems_t_p(book);
	    ::sim_mob::xml::BusStop_t_pimpl BusStop_t_p;
	    ::xml_schema::double_pimpl double_p;
	    ::sim_mob::xml::ERP_Gantry_t_pimpl ERP_Gantry_t_p;
	    ::sim_mob::xml::crossing_t_pimpl crossing_t_p;
	    ::sim_mob::xml::PointPair_t_pimpl PointPair_t_p;
	    ::sim_mob::xml::RoadBump_t_pimpl RoadBump_t_p;
	    ::sim_mob::xml::TripChain_t_pimpl TripChain_t_p;
	    ::xml_schema::integer_pimpl integer_p;
	    ::sim_mob::xml::TripchainItemType_pimpl TripchainItemType_p;
	    ::sim_mob::xml::TripchainItemLocationType_pimpl TripchainItemLocationType_p;
	    ::sim_mob::xml::SubTrips_t_pimpl SubTrips_t_p;
	    ::sim_mob::xml::Signals_t_pimpl Signals_t_p;
	    ::sim_mob::xml::Signal_t_pimpl Signal_t_p(book);
	    ::xml_schema::unsigned_byte_pimpl unsigned_byte_p;
//	    ::sim_mob::xml::signalTimingMode_t_pimpl signalAlgorithm_t_p;
	    ::sim_mob::xml::linkAndCrossings_t_pimpl linkAndCrossings_t_p;
	    ::sim_mob::xml::SplitPlan_t_pimpl SplitPlan_t_p;
	    ::sim_mob::xml::Plans_t_pimpl Plans_t_p;
	    ::sim_mob::xml::Plan_t_pimpl Plan_t_p;
	    ::sim_mob::xml::Phases_t_pimpl Phases_t_p;
	    ::sim_mob::xml::Phase_t_pimpl Phase_t_p;
	    ::sim_mob::xml::links_maps_t_pimpl links_maps_t_p;
	    ::sim_mob::xml::links_map_t_pimpl links_map_t_p(book);
	    ::sim_mob::xml::ColorSequence_t_pimpl ColorSequence_t_p;
	    ::sim_mob::xml::ColorDuration_t_pimpl ColorDuration_t_p;
	    ::sim_mob::xml::TrafficColor_t_pimpl TrafficColor_t_p;
	    ::sim_mob::xml::crossings_maps_t_pimpl crossings_maps_t_p/*(book)*/;
	    ::sim_mob::xml::crossings_map_t_pimpl crossings_map_t_p(book);
	    ::sim_mob::xml::SCATS_t_pimpl SCATS_t_p/*(book,signal)*/;
	    ::sim_mob::xml::signalTimingMode_t_pimpl signalTimingMode_t_p;	    

	    // Connect the parsers together.
	    //
	    SimMobility_t_p.parsers (GeoSpatial_t_p,
	                             TripChains_t_p,
	                             Signals_t_p);

	    GeoSpatial_t_p.parsers (RoadNetwork_t_p);

	    RoadNetwork_t_p.parsers (coordinate_map_t_p,
                                 road_runner_regions_t_p,
	                             Nodes_p,
	                             Links_p);

	    coordinate_map_t_p.parsers (utm_projection_t_p,
	                                linear_scale_t_p);

	    utm_projection_t_p.parsers (string_p,
	                                string_p);

	    linear_scale_t_p.parsers (scale_source_t_p,
	                              scale_destination_t_p);

	    scale_source_t_p.parsers (string_p,
	                              string_p);

	    scale_destination_t_p.parsers (string_p,
	                                   string_p);

	    road_runner_regions_t_p.parsers(roadrunner_region_t_p);
	    roadrunner_region_t_p.parsers(int_p, roadrunner_shape_t_p);
	    roadrunner_shape_t_p.parsers(roadrunner_vertex_t_p);
	    roadrunner_vertex_t_p.parsers(double_p, double_p);

	    Nodes_p.parsers (UniNodes_p,
	                     Intersections_p,
	                     roundabouts_p);

	    UniNodes_p.parsers (UniNode_t_p);

	    UniNode_t_p.parsers (unsigned_int_p,
	                         Point2D_t_p,
	                         string_p,
	                         temp_Segmetair_t_p,
	                         temp_Segmetair_t_p,
	                         connectors_t_p);

	    Point2D_t_p.parsers (unsigned_int_p,
	                         unsigned_int_p);

	    temp_Segmetair_t_p.parsers (unsigned_long_p,
	                                unsigned_long_p);

	    connectors_t_p.parsers (connector_t_p);

	    connector_t_p.parsers (unsigned_long_p,
	                           unsigned_long_p);

	    Intersections_p.parsers (intersection_t_p);

	    intersection_t_p.parsers (unsigned_int_p,
	                              Point2D_t_p,
	                              string_p,
	                              RoadSegmentsAt_t_p,
	                              Multi_Connectors_t_p,
	                              ChunkLengths_t_p,
	                              offsets_t_p,
	                              separators_t_p,
	                              LanesVector_t_p,
	                              LanesVector_t_p,
	                              DomainIslands_t_p);

	    RoadSegmentsAt_t_p.parsers (unsigned_long_p);

	    Multi_Connectors_t_p.parsers (Multi_Connector_t_p);

	    Multi_Connector_t_p.parsers (unsigned_long_p,
	                                 connectors_t_p);

	    ChunkLengths_t_p.parsers (ChunkLength_t_p);

	    ChunkLength_t_p.parsers (unsigned_short_p,
	                             unsigned_int_p);

	    offsets_t_p.parsers (offset_t_p);

	    offset_t_p.parsers (unsigned_short_p,
	                        unsigned_int_p);

	    separators_t_p.parsers (separator_t_p);

	    separator_t_p.parsers (unsigned_short_p,
	                           boolean_p);

	    LanesVector_t_p.parsers (unsigned_long_p);

	    DomainIslands_t_p.parsers (DomainIsland_t_p);

	    DomainIsland_t_p.parsers (unsigned_short_p,
	                              boolean_p);

	    roundabouts_p.parsers (roundabout_t_p);

	    roundabout_t_p.parsers (unsigned_int_p,
	                            Point2D_t_p,
	                            string_p,
	                            RoadSegmentsAt_t_p,
	                            Multi_Connectors_t_p,
	                            ChunkLengths_t_p,
	                            offsets_t_p,
	                            separators_t_p,
	                            LanesVector_t_p,
	                            float_p,
	                            int_p,
	                            EntranceAngles_t_p);

	    EntranceAngles_t_p.parsers (EntranceAngle_t_p);

	    EntranceAngle_t_p.parsers (unsigned_short_p,
	                               unsigned_int_p);

	    Links_p.parsers (link_t_p);

	    link_t_p.parsers (unsigned_int_p,
	                      string_p,
	                      unsigned_int_p,
	                      unsigned_int_p,
	                      fwdBckSegments_t_p);

	    fwdBckSegments_t_p.parsers (segment_t_p);

	    segment_t_p.parsers (unsigned_long_p,
	                         unsigned_int_p,
	                         unsigned_int_p,
	                         short_p,
	                         unsigned_int_p,
	                         unsigned_int_p,
	                         string_p,
	                         PolyLine_t_p,
	                         laneEdgePolylines_cached_t_p,
	                         Lanes_p,
	                         RoadItems_t_p,
	                         PolyLine_t_p);

	    PolyLine_t_p.parsers (PolyPoint_t_p);

	    PolyPoint_t_p.parsers (string_p,
	                           Point2D_t_p);

	    laneEdgePolylines_cached_t_p.parsers (laneEdgePolyline_cached_t_p);

	    laneEdgePolyline_cached_t_p.parsers (short_p,
	                                         PolyLine_t_p);

	    Lanes_p.parsers (lane_t_p);

	    lane_t_p.parsers (unsigned_long_p,
	                      unsigned_int_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      boolean_p,
	                      PolyLine_t_p);

	    RoadItems_t_p.parsers (BusStop_t_p,
	                           ERP_Gantry_t_p,
	                           crossing_t_p,
	                           RoadBump_t_p);

	    BusStop_t_p.parsers (unsigned_long_p,
	                         unsigned_short_p,
	                         Point2D_t_p,
	                         Point2D_t_p,
	                         double_p,
	                         double_p,
	                         unsigned_long_p,
	                         boolean_p,
	                         boolean_p,
	                         boolean_p,
	                         unsigned_int_p,
	                         string_p);

	    ERP_Gantry_t_p.parsers (unsigned_long_p,
	                            unsigned_short_p,
	                            Point2D_t_p,
	                            Point2D_t_p,
	                            string_p);

	    crossing_t_p.parsers (unsigned_long_p,
	                          unsigned_short_p,
	                          Point2D_t_p,
	                          Point2D_t_p,
	                          PointPair_t_p,
	                          PointPair_t_p);

	    PointPair_t_p.parsers (Point2D_t_p,
	                           Point2D_t_p);

	    RoadBump_t_p.parsers (unsigned_long_p,
	                          unsigned_short_p,
	                          Point2D_t_p,
	                          Point2D_t_p,
	                          string_p,
	                          unsigned_long_p);

	    TripChains_t_p.parsers (TripChain_t_p);

	    TripChain_t_p.parsers (string_p,
	                           Trip_t_p,
	                           Activity_t_p);

	    Trip_t_p.parsers (string_p,
	                      TripchainItemType_p,
	                      unsigned_int_p,
	                      integer_p,
	                      string_p,
	                      string_p,
	                      integer_p,
	                      unsigned_int_p,
	                      TripchainItemLocationType_p,
	                      unsigned_int_p,
	                      TripchainItemLocationType_p,
	                      SubTrips_t_p);

	    SubTrips_t_p.parsers (SubTrip_t_p);

	    SubTrip_t_p.parsers (string_p,
	                         TripchainItemType_p,
	                         unsigned_int_p,
	                         integer_p,
	                         string_p,
	                         string_p,
	                         integer_p,
	                         unsigned_int_p,
	                         TripchainItemLocationType_p,
	                         unsigned_int_p,
	                         TripchainItemLocationType_p,
	                         SubTrips_t_p,
	                         string_p,
	                         boolean_p,
	                         string_p);

	    Activity_t_p.parsers (string_p,
	                          TripchainItemType_p,
	                          unsigned_int_p,
	                          integer_p,
	                          string_p,
	                          string_p,
	                          string_p,
	                          unsigned_int_p,
	                          TripchainItemLocationType_p,
	                          boolean_p,
	                          boolean_p,
	                          boolean_p);


	    Signals_t_p.parsers (Signal_t_p);

	    Signal_t_p.parsers (unsigned_int_p,
	                        unsigned_int_p,
	                        linkAndCrossings_t_p,
	                        Phases_t_p,
	                        SCATS_t_p);

	    linkAndCrossings_t_p.parsers (linkAndCrossing_t_p);

	    linkAndCrossing_t_p.parsers (unsigned_byte_p,
	                                 unsigned_int_p,
	                                 unsigned_int_p,
	                                 unsigned_byte_p);

	    Phases_t_p.parsers (Phase_t_p);

	    Phase_t_p.parsers (unsigned_byte_p,
	                       string_p,
	                       links_maps_t_p,
	                       crossings_maps_t_p);

	    links_maps_t_p.parsers (links_map_t_p);

	    links_map_t_p.parsers (unsigned_int_p,
	                           unsigned_int_p,
	                           unsigned_int_p,
	                           unsigned_int_p,
	                           ColorSequence_t_p);

	    ColorSequence_t_p.parsers (string_p,
	                               ColorDuration_t_p);

	    ColorDuration_t_p.parsers (TrafficColor_t_p,
	    		unsigned_byte_p);

	    crossings_maps_t_p.parsers (crossings_map_t_p);

	    crossings_map_t_p.parsers (unsigned_int_p,
	                               unsigned_int_p,
	                               ColorSequence_t_p);

	    SCATS_t_p.parsers (signalTimingMode_t_p,
	                       SplitPlan_t_p);

	    SplitPlan_t_p.parsers (unsigned_int_p,
	                           unsigned_byte_p,
	                           unsigned_byte_p,
	                           Plans_t_p);

	    Plans_t_p.parsers (Plan_t_p);

	    Plan_t_p.parsers (unsigned_byte_p,
	                      double_p);

	    bool customSchema = false;
	    xml_schema::properties props;
	    if (!sim_mob::ConfigManager::GetInstance().FullConfig().roadNetworkXsdSchemaFile().empty()) {
	    	customSchema = true;
	    	//props.no_namespace_schema_location(sim_mob::ConfigParams::GetInstance().roadNetworkXsdSchemaFile);
	    	props.schema_location ("http://www.smart.mit.edu/geo", sim_mob::ConfigManager::GetInstance().FullConfig().roadNetworkXsdSchemaFile());
	    }

		//Parse differently depending on what we are trying to fill.
	    if (resultNetwork && resultTripChains) {
	    	//Parse the entire thing.
			::xml_schema::document doc_p(SimMobility_t_p, "http://www.smart.mit.edu/geo", rootNode);
			SimMobility_t_p.pre();
			if (customSchema) {
				doc_p.parse(fileName, 0, props);
			} else {
				doc_p.parse(fileName);
			}
			SimMobility_t_p.post_SimMobility_t();
	    } else if (!resultNetwork && resultTripChains) {
	    	//Only parse the tripchains
			::xml_schema::document doc_p(TripChains_t_p, "http://www.smart.mit.edu/geo", rootNode);
			TripChains_t_p.pre();
			doc_p.parse(fileName);
			TripChains_t_p.post_TripChains_t();
	    } else {
	    	throw std::runtime_error("geo10-driver sanity check failed; unknown parameter combination.");
	    }

	} catch (const ::xml_schema::exception& e) {
		std::cerr << e << std::endl;
		return false;
	} catch (const std::ios_base::failure&) {
		std::cerr <<fileName << ": error: io failure" << std::endl;
		return false;
	}

	return true;
}

} //End unnamed namespace


bool sim_mob::xml::InitAndLoadXML(const std::string& fileName, sim_mob::RoadNetwork& resultNetwork, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& resultTripChains)
{
	return init_and_load_internal(fileName, "SimMobility", &resultNetwork, &resultTripChains);
}

bool sim_mob::xml::InitAndLoadTripChainsFromXML(const std::string& fileName, const std::string& rootNode, std::map<std::string, std::vector<sim_mob::TripChainItem*> >& resultTripChains)
{
	return init_and_load_internal(fileName, rootNode, nullptr, &resultTripChains);
}





