#include "auto_complete_graph/ACG.hpp"

g2o::VertexSE2* AASS::acg::AutoCompleteGraph::addRobotPose(const g2o::SE2& se2){
	
	std::cout << "Adding the robot pose " << std::endl;
	g2o::VertexSE2* robot =  new g2o::VertexSE2;
	robot->setEstimate(se2);
	robot->setId(_optimizable_graph.vertices().size());
	_optimizable_graph.addVertex(robot);
	_nodes_ndt.push_back(robot);
	return robot;
}
g2o::VertexSE2* AASS::acg::AutoCompleteGraph::addRobotPose(const Eigen::Vector3d& rob){
	g2o::SE2 se2(rob(0), rob(1), rob(2));
	return addRobotPose(se2);
}
g2o::VertexSE2* AASS::acg::AutoCompleteGraph::addRobotPose(double x, double y, double theta){
	Eigen::Vector3d robot1;
	robot1 << x, y, theta;
	return addRobotPose(robot1);
}

g2o::VertexPointXY* AASS::acg::AutoCompleteGraph::addLandmarkPose(const g2o::Vector2D& pos, int strength){
	g2o::VertexPointXY* landmark = new g2o::VertexPointXY;
	landmark->setId(_optimizable_graph.vertices().size());
	landmark->setEstimate(pos);
	_optimizable_graph.addVertex(landmark);
	_nodes_landmark.push_back(landmark);
	return landmark;
}
g2o::VertexPointXY* AASS::acg::AutoCompleteGraph::addLandmarkPose(double x, double y, int strength){
	g2o::Vector2D lan;
	lan << x, y;
	return addLandmarkPose(lan, strength);
}

AASS::acg::VertexPrior* AASS::acg::AutoCompleteGraph::addPriorLandmarkPose(const g2o::SE2& se2){
	AASS::acg::VertexPrior* priorlandmark = new AASS::acg::VertexPrior;
	priorlandmark->setId(_optimizable_graph.vertices().size());
	priorlandmark->setEstimate(se2);
	_optimizable_graph.addVertex(priorlandmark);
	_nodes_prior.push_back(priorlandmark);
	return priorlandmark;
}
AASS::acg::VertexPrior* AASS::acg::AutoCompleteGraph::addPriorLandmarkPose(const Eigen::Vector3d& lan){
	g2o::SE2 se2(lan(0), lan(1), lan(2));
	return addPriorLandmarkPose(se2);
}
AASS::acg::VertexPrior* AASS::acg::AutoCompleteGraph::addPriorLandmarkPose(double x, double y, double theta){
	Eigen::Vector3d lan;
	lan << x, y, theta;
	return addPriorLandmarkPose(lan);
}


void AASS::acg::AutoCompleteGraph::addOdometry(const g2o::SE2& se2, g2o::HyperGraph::Vertex* v1, g2o::HyperGraph::Vertex* v2){
			
	Eigen::Matrix3d covariance;
	covariance.fill(0.);
	covariance(0, 0) = _transNoise[0]*_transNoise[0];
	covariance(1, 1) = _transNoise[1]*_transNoise[1];
	covariance(2, 2) = _rotNoise*_rotNoise;
	Eigen::Matrix3d information = covariance.inverse();
	
	g2o::EdgeSE2* odometry = new g2o::EdgeSE2;
	odometry->vertices()[0] = v1 ;
	odometry->vertices()[1] = v2 ;
	odometry->setMeasurement(se2);
	odometry->setInformation(information);
	_optimizable_graph.addEdge(odometry);
	_edge_odometry.push_back(odometry);
}
void AASS::acg::AutoCompleteGraph::addOdometry(const g2o::SE2& observ, int from, int toward){
	g2o::HyperGraph::Vertex* from_ptr = _optimizable_graph.vertex(from);
	g2o::HyperGraph::Vertex* toward_ptr = _optimizable_graph.vertex(toward);
	addOdometry(observ, from_ptr, toward_ptr);
}
void AASS::acg::AutoCompleteGraph::addOdometry(double x, double y, double theta, int from, int toward){
	g2o::SE2 se2(x, y, theta);
	addOdometry(se2, from, toward);
}

void AASS::acg::AutoCompleteGraph::addLandmarkObservation(const g2o::Vector2D& pos, g2o::HyperGraph::Vertex* v1, g2o::HyperGraph::Vertex* v2){
	Eigen::Matrix2d covariance_landmark; 
	covariance_landmark.fill(0.);
	covariance_landmark(0, 0) = _landmarkNoise[0]*_landmarkNoise[0];
	covariance_landmark(1, 1) = _landmarkNoise[1]*_landmarkNoise[1];
// 			covariance_landmark(2, 2) = 13;//<- Rotation covariance landmark is more than 4PI
	Eigen::Matrix2d information_landmark = covariance_landmark.inverse();
	
	g2o::EdgeSE2PointXY* landmarkObservation =  new g2o::EdgeSE2PointXY;
	landmarkObservation->vertices()[0] = v1;
	landmarkObservation->vertices()[1] = v2;
	landmarkObservation->setMeasurement(pos);
	landmarkObservation->setInformation(information_landmark);
	landmarkObservation->setParameterId(0, _sensorOffset->id());
	_optimizable_graph.addEdge(landmarkObservation);
	_edge_landmark.push_back(landmarkObservation);
}
void AASS::acg::AutoCompleteGraph::addLandmarkObservation(const g2o::Vector2D& pos, int from, int toward){
	g2o::HyperGraph::Vertex* from_ptr = _optimizable_graph.vertex(from);
	g2o::HyperGraph::Vertex* toward_ptr = _optimizable_graph.vertex(toward);
	addLandmarkObservation(pos, from_ptr, toward_ptr);
}

void AASS::acg::AutoCompleteGraph::addEdgePrior(const g2o::SE2& se2, g2o::HyperGraph::Vertex* v1, g2o::HyperGraph::Vertex* v2){
	
	
	//Get Eigen vector
	VertexPrior* v_ptr = dynamic_cast<VertexPrior*>(v1);
	VertexPrior* v_ptr2 = dynamic_cast<VertexPrior*>(v2);
	Eigen::Vector3d pose1 = v_ptr->estimate().toVector();
	Eigen::Vector3d pose2 = v_ptr2->estimate().toVector();
	
// 			std::cout << "Poses 1 " << std::endl << pose1.format(cleanFmt) << std::endl;
// 			std::cout << "Poses 2 " << std::endl << pose2.format(cleanFmt) << std::endl;
	
	Eigen::Vector2d eigenvec;
	eigenvec << pose1(0) - pose2(0), pose1(1) - pose2(1);
// 				std::cout << "EigenVec " << std::endl << eigenvec.format(cleanFmt) << std::endl;
	std::pair<double, double> eigenval(_priorNoise(0), _priorNoise(1));
	
	Eigen::Matrix2d cov = ndt_feature::getCovarianceVec(eigenvec, eigenval);
	
// 			std::cout << "Covariance prior " << std::endl << cov.format(cleanFmt) << std::endl;
	
	Eigen::Matrix3d covariance_prior;
	covariance_prior.fill(0.);
	covariance_prior(0, 0) = cov(0, 0);
	covariance_prior(0, 1) = cov(0, 1);
	covariance_prior(1, 0) = cov(1, 0);
	covariance_prior(1, 1) = cov(1, 1);
	covariance_prior(2, 2) = 13;//<- Rotation covariance prior landmark is more than 4PI
	Eigen::Matrix3d information_prior = covariance_prior.inverse();
// 			std::cout << "Information prior " << std::endl << cov.format(cleanFmt) << std::endl;
	
	
	
	g2o::EdgeSE2Prior_malcolm* priorObservation =  new g2o::EdgeSE2Prior_malcolm;
	priorObservation->vertices()[0] = v1;
	priorObservation->vertices()[1] = v2;
	priorObservation->setMeasurement(se2);
	priorObservation->setInformation(information_prior);
	priorObservation->setParameterId(0, _sensorOffset->id());
	_optimizable_graph.addEdge(priorObservation);
	_edge_prior.push_back(priorObservation);
}

// void AASS::acg::AutoCompleteGraph::addEdgePrior(g2o::SE2 observ, int from, int toward){
// 	std::tuple<g2o::SE2, int, int> obs1(observ, from, toward);
// 	addEdgePrior(obs1);
// }
// void AASS::acg::AutoCompleteGraph::addEdgePrior(double x, double y, double theta, int from, int toward){
// 	g2o::SE2 se2(x, y, theta);
// 	addEdgePrior(se2, from, toward);
// 	
// }

void AASS::acg::AutoCompleteGraph::addLinkBetweenMaps(const g2o::Vector2D& pos, g2o::HyperGraph::Vertex* v1, g2o::HyperGraph::Vertex* v2){
	Eigen::Matrix2d covariance_link; 
	covariance_link.fill(0.);
	covariance_link(0, 0) = _linkNoise[0]*_linkNoise[0];
	covariance_link(1, 1) = _linkNoise[1]*_linkNoise[1];
// 			covariance_link(2, 2) = 13;//<- Rotation covariance link is more than 4PI
	Eigen::Matrix2d information_link = covariance_link.inverse();
	
	g2o::EdgeLinkXY_malcolm* linkObservation =  new g2o::EdgeLinkXY_malcolm;
	linkObservation->vertices()[0] = v1;
	linkObservation->vertices()[1] = v2;
	linkObservation->setMeasurement(pos);
	linkObservation->setInformation(information_link);
	linkObservation->setParameterId(0, _sensorOffset->id());
	_optimizable_graph.addEdge(linkObservation);
	_edge_link.push_back(linkObservation);
}
void AASS::acg::AutoCompleteGraph::addLinkBetweenMaps(const g2o::Vector2D& pos, int from, int toward){
	g2o::HyperGraph::Vertex* from_ptr = _optimizable_graph.vertex(from);
	g2o::HyperGraph::Vertex* toward_ptr = _optimizable_graph.vertex(toward);
	addLinkBetweenMaps(pos, from_ptr, toward_ptr);
}


//FUNCTION TO REMOVE A VERTEX
void AASS::acg::AutoCompleteGraph::removeVertex(g2o::HyperGraph::Vertex* v1){
	//Prior
	AASS::acg::VertexPrior* ptr = dynamic_cast<AASS::acg::VertexPrior*>(v1);
	g2o::VertexSE2* ptr_se2 = dynamic_cast<g2o::VertexSE2*>(v1);
	g2o::VertexPointXY* ptr_se3 = dynamic_cast<g2o::VertexPointXY*>(v1);
	
	if(ptr != NULL){
		int index = findPriorNode(v1);
		assert(index != -1);
		std::vector<AASS::acg::VertexPrior*>::iterator which = _nodes_prior.begin() + index;
		_nodes_prior.erase(which);
	}
	//Robot node
	else if( ptr_se2 != NULL){
		int index = findRobotNode(v1);
		assert(index != -1);
		std::vector<g2o::VertexSE2*>::iterator which = _nodes_ndt.begin() + index;
		_nodes_ndt.erase(which);
		
	}
	//Landmark Node
	else if( ptr_se3 != NULL){
		int index = findLandmarkNode(v1);
		assert(index != -1);
		std::vector<g2o::VertexPointXY*>::iterator which = _nodes_landmark.begin() + index;
		_nodes_landmark.erase(which);
	}
	else{
		throw std::runtime_error("Vertex type not found in list");
	}
	_optimizable_graph.removeVertex(v1, true);
}

int AASS::acg::AutoCompleteGraph::findRobotNode(g2o::HyperGraph::Vertex* v){
	int pos = 0;
	auto it = _nodes_ndt.begin();
	for(it ; it != _nodes_ndt.end() ; ++it){
		if(*it == v){
			return pos;
		}
		++pos;
	}
	return -1;
}
int AASS::acg::AutoCompleteGraph::findLandmarkNode(g2o::HyperGraph::Vertex* v){
	int pos = 0;
	auto it = _nodes_landmark.begin();
	for(it ; it != _nodes_landmark.end() ; ++it){
		if(*it == v){
			return pos;
		}
		++pos;
	}
	return -1;
	
}
int AASS::acg::AutoCompleteGraph::findPriorNode(g2o::HyperGraph::Vertex* v){
	int pos = 0;
	auto it = _nodes_prior.begin();
	for(it ; it != _nodes_prior.end() ; ++it){
		if(*it == v){
			return pos;
		}
		++pos;
	}
	return -1;
	
}

void AASS::acg::AutoCompleteGraph::addPriorGraph(const bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>& graph){
	
	std::pair< bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>::VertexIterator, bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>::VertexIterator > vp;
	std::deque<bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>::Vertex> vec_deque;
	std::vector<AASS::acg::VertexPrior*> out_prior;
	
	for (vp = boost::vertices(graph); vp.first != vp.second; ++vp.first) {
		bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>::Vertex v = *vp.first;
		//ATTENTION Magic number
		AASS::acg::VertexPrior* res = addPriorLandmarkPose(graph[v].getX(), graph[v].getY(), 0);
		vec_deque.push_back(v);
		out_prior.push_back(res);
	}
	
	int count = 0;
	for (vp = boost::vertices(graph); vp.first != vp.second; ++vp.first) {
		bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>::Vertex v = *vp.first;
		bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>::EdgeIterator out_i, out_end;
		bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>::Edge e;
		
		for (boost::tie(out_i, out_end) = boost::out_edges(v, (graph)); 
			out_i != out_end; ++out_i) {
			e = *out_i;
			bettergraph::PseudoGraph<AASS::vodigrex::SimpleNode, AASS::vodigrex::SimpleEdge>::Vertex targ = boost::target(e, (graph));
		
			int idx = -1;
			for(size_t ii = count +1 ; ii < vec_deque.size() ; ++ii){
				if(targ == vec_deque[ii]){
					idx = ii;
				}
			}
			if(idx == -1){
				//SKIP
			}
			else{
				AASS::acg::VertexPrior* from = out_prior[count];
				AASS::acg::VertexPrior* toward = out_prior[idx];
				int x_diff = graph[targ].getX() - graph[v].getX();
				int y_diff = graph[targ].getY() - graph[v].getY();
				g2o::SE2 se2(x_diff, y_diff, 0);
				addEdgePrior(se2, from, toward);
			}
		
		}
		++count;
	}
	
	std::cout << _edge_prior.size() << " == " << graph.getNumEdges() << std::endl;
	assert( _nodes_landmark.size() == graph.getNumVertices() );
	assert( _edge_prior.size() == graph.getNumEdges());
	
}

void AASS::acg::AutoCompleteGraph::updateNDTGraph(ndt_feature::NDTFeatureGraph& ndt_graph){
	
	std::vector<NDTCornerGraphElement> corners_end;
	double cell_size = 0;
	
	std::cout << "Should we check " <<ndt_graph.getNbNodes() << ">" << _previous_number_of_node_in_ndtgraph << std::endl;
	
	if(ndt_graph.getNbNodes() > _previous_number_of_node_in_ndtgraph){
		
		std::cout << "Found new nodes" << std::endl;
		
		size_t i;
		auto links = ndt_graph.getOdometryLinks();
		ndt_graph.updateLinksUsingNDTRegistration(links, 10, true);
		
		//Should most of the time be one but just in case it runs slowly I'll let that
		if(_previous_number_of_node_in_ndtgraph != 0){
			i = _previous_number_of_node_in_ndtgraph - 1;
// 			for (size_t i = 0 ; i <links.size() ; ++i) {
// 				Eigen::IOFormat cleanFmt(4, 0, ", ", "\n", "[", "]");
// 				std::cout <<"Estimate before anything " << links[i].getRelCov().inverse().format(cleanFmt) << std::endl;
// 				std::cout << "Adding Edge" << std::endl;
// // 				NDTFeatureLink link = NDTFeatureLink((const NDTFeatureLink&) graph.getLinkInterface(i));
// 				g2o::SE2 odometry = ndt_feature::NDTFeatureLink2EdgeSE2(links[i]);
// 				size_t from = links[i].getRefIdx() ;
// 				size_t toward = links[i].getMovIdx() ;
// 				std::cout << "from " << from << " toward " << toward << std::endl;
// 				addOdometry(odometry, from, toward);
// 			}
			
			
		}
		else{
			i = 0;
		}
		for (i; i < ndt_graph.getNbNodes() - 1; ++i) {
			
			std::cout << "Checking node nb " << i << std::endl;
			//RObot pose
			ndt_feature::NDTFeatureNode* feature = new ndt_feature::NDTFeatureNode();
			std::cout << "Copy feature" << std::endl;
			feature->copyNDTFeatureNode( (const ndt_feature::NDTFeatureNode&)ndt_graph.getNodeInterface(i) );
			Eigen::Affine3d affine = Eigen::Affine3d(feature->getPose());
			Eigen::Isometry2d isometry2d = ndt_feature::Affine3d2Isometry2d(affine);
			g2o::SE2 robot_pos(isometry2d);
			
// 			Eigen::Vector2d robot_pos; robot_pos << robot_pos_tmp(0), robot_pos_tmp(1);
// 			robot_pos << ndt_graph.getNode(i).T(0), ndt_graph.getNode(i).T(1);
			
			g2o::VertexSE2* robot_ptr = addRobotPose(robot_pos);
			//Add Odometry
			if(i > 0 ){
				std::cout << "adding the odometry" << std::endl;
				g2o::SE2 odometry = ndt_feature::NDTFeatureLink2EdgeSE2(links[i - 1]);
				std::cout << " ref " << links[i-1].getRefIdx() << " and mov " << links[i-1].getMovIdx() << std::endl;
				assert( links[i-1].getRefIdx() < _nodes_ndt.size() );
				assert( links[i-1].getMovIdx() < _nodes_ndt.size() );
				auto from = _nodes_ndt[ links[i-1].getRefIdx() ] ;
				auto toward = _nodes_ndt[ links[i-1].getMovIdx() ] ;
				addOdometry(odometry, from, toward);
			}
			
			
			lslgeneric::NDTMap* map = ndt_graph.getMap(i);
			
			//HACK For now : we translate the Corner extracted and not the ndt-maps
			auto cells = map->getAllCells();
			double x2, y2, z2;
			map->getCellSizeInMeters(x2, y2, z2);
			cell_size = x2;
			
			AASS::das::NDTCorner cornersExtractor;
// 					std::cout << "Searching for corners in map with " << cells.size() << " initialized cells, and celle size is " << x2 << " " << y2 << " " << z2 << std::endl;
			auto ret_export = cornersExtractor.getAllCorners(*map);
			auto ret_opencv_point_corner = cornersExtractor.getAccurateCvCorners();			
// 					std::cout << "Corner extracted. Nb of them " << ret_opencv_point_corner.size() << std::endl;
			
			//HACK: translate the corners now :
			auto it = ret_opencv_point_corner.begin();
			
			std::cout << "Found " << ret_opencv_point_corner.size() << " corners " << std::endl;
			
			//Find all the observations :

			for(it ; it != ret_opencv_point_corner.end() ; ++it){
// 						std::cout << "MOVE : "<< it -> x << " " << it-> y << std::endl;
				Eigen::Vector3d vec;
				vec << it->x, it->y, 0;
				
				//Wrong because we need to take in account the orientation
// 				Eigen::Vector3d vec;
// 				vec << it->x, it->y, 0;
				
				
				Eigen::Vector3d vec_out = ndt_graph.getNode(i).T * vec;
				cv::Point2f p_out(vec_out(0), vec_out(1));
				
// 				g2o::Vector2D observation;
// 				observation << it->x - p_out.x, it->y  - p_out.y ;
				
				Eigen::Vector2d real_obs; real_obs << p_out.x, p_out.y;
				Eigen::Vector2d observation;
				//Projecting real_obs into robot coordinate frame
				Eigen::Vector2d trueObservation = robot_pos.inverse() * real_obs;
				observation = trueObservation;

				
				std::cout << "Node transfo " << ndt_graph.getNode(i).T.matrix() << std::endl;
				std::cout << "Position node " << robot_pos.toVector() << std::endl;
				std::cout << " vec " << vec << std::endl;
				std::cout << "Well " << robot_pos.toVector() + vec << "==" << ndt_graph.getNode(i).T * vec << std::endl;
				
				//ATTENTION THIS IS NOT TRUE BUT REALLY CLOSE
// 				assert (robot_pos + vec == ndt_graph.getNode(i).T * vec);
				
// 				std::cout << "NEW POINT : "<< p_out << std::endl;
				
				NDTCornerGraphElement cor(p_out);
				cor.addAllObserv(i, robot_ptr, observation);
				corners_end.push_back(cor);
// 						cor.addNode(i);
// 						cor.nodes_linked_ptr.push_back(robot_ptr);
// 						cor.addObservation(observation);
				
			}
			//At this point, we have all the corners
		}
		//Save new number of nodes to update
		
		
	}		
	
	std::vector<g2o::VertexPointXY*> all_new_landmarks;
	
	//Add stuff directly in the optimization graph : 
	for(size_t i = 0 ; i < corners_end.size() ; ++i){
		std::cout << "checking corner : " ;  corners_end[i].print() ; std::cout << std::endl;	
		bool seen = false;
		g2o::VertexPointXY* ptr_landmark_seen = NULL;
		for(size_t j = 0 ; j <_nodes_landmark.size() ; ++j){
// 			if(tmp[j] == _corners_position[i]){
			g2o::Vector2D landmark = _nodes_landmark[j]->estimate();
			cv::Point2f point_land(landmark(0), landmark(1));
			
			double res = cv::norm(point_land - corners_end[i].point);
			
			std::cout << "res : " << res << " points "  << point_land << " " << corners_end[i].point << "  cell size " << cell_size << std::endl;
			
			//If we found the landmark, we save the data
			if( res < cell_size){
				seen = true;
				ptr_landmark_seen = _nodes_landmark[j];
			}
		}
		if(seen == false){
			std::cout << "New point" << std::endl;
			g2o::Vector2D vec;
			vec << corners_end[i].point.x, corners_end[i].point.y ;
			g2o::VertexPointXY* ptr = addLandmarkPose(vec, 1);
// 			g2o::VertexPointXY* ptr;
			all_new_landmarks.push_back(ptr);
			//Adding all links
			for(int no = 0 ; no < corners_end[i].getNodeLinked().size() ; ++no){
// 						addLandmarkObservation(corners_end[i].getObservations()[no], _nodes_ndt[corners_end[i].getNodeLinked()[no], ptr);
				addLandmarkObservation(corners_end[i].getObservations()[no], corners_end[i].getNodeLinkedPtr()[no], ptr);
			}
		}
		else{
			std::cout << "Point seen " << std::endl;
			for(int no = 0 ; no < corners_end[i].getNodeLinked().size() ; ++no){
				addLandmarkObservation(corners_end[i].getObservations()[no], corners_end[i].getNodeLinkedPtr()[no], ptr_landmark_seen);
			}				
		}
	}
	_previous_number_of_node_in_ndtgraph = ndt_graph.getNbNodes();
	
	
	//TODO, update links using the newly found landmarks. No need to update the rest obviously
	
	updateLinksAfterNDTGraph(all_new_landmarks);

	
}


void AASS::acg::AutoCompleteGraph::updateLinksAfterNDTGraph(const std::vector<g2o::VertexPointXY*>& new_landmarks)
{
	std::vector < std::pair < g2o::VertexPointXY*, AASS::acg::VertexPrior*> > links;
	
	std::cout << "Number new landmarks " << new_landmarks.size() << std::endl;
	std::cout << "Prior " << _nodes_prior.size() << std::endl;
	if(_nodes_prior.size() > 0){
		auto it = new_landmarks.begin();
		for(it ; it != new_landmarks.end() ; it++){
			std::cout << "Working on links " << std::endl;
			Eigen::Vector2d pose_landmark = (*it)->estimate();
			auto it_prior = _nodes_prior.begin();
			
			Eigen::Vector3d pose_tmp = (*it_prior)->estimate().toVector();
			Eigen::Vector2d pose_prior; pose_prior << pose_tmp(0), pose_tmp(1);
			
			double norm = (pose_prior - pose_landmark).norm();
			AASS::acg::VertexPrior* ptr_closest = *it_prior;
			
			for(it_prior ; it_prior != _nodes_prior.end() ; ++it_prior){
				pose_tmp = (*it_prior)->estimate().toVector();
				pose_prior << pose_tmp(0), pose_tmp(1);
				double norm_tmp = (pose_prior - pose_landmark).norm();
				
				//Update the link
				if(norm_tmp < norm){
					ptr_closest = *it_prior;
				}			
			}
			//Pushing the link
			links.push_back(std::pair<g2o::VertexPointXY*, AASS::acg::VertexPrior*>(*it, ptr_closest));
		}
		
		
		auto it_links = links.begin();
		for(it_links ; it_links != links.end() ; it_links++){
			g2o::SE2 se2(0, 0, 0);
			addEdgePrior(se2, it_links->second, it_links->first);
		}
	}

}