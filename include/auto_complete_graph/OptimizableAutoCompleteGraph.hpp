#ifndef AUTOCOMPLETEGRAPH_OPTIMIZABLEGRAPH_07102016
#define AUTOCOMPLETEGRAPH_OPTIMIZABLEGRAPH_07102016


#include "g2o/types/slam2d/parameter_se2_offset.h"
#include "g2o/core/sparse_optimizer.h"
#include "g2o/core/block_solver.h"
#include "g2o/core/factory.h"
#include "g2o/core/optimization_algorithm_factory.h"
#include "g2o/core/optimization_algorithm_gauss_newton.h"
#include "g2o/solvers/csparse/linear_solver_csparse.h"
#include "g2o/core/robust_kernel_impl.h"

namespace AASS {

namespace acg{	
	
	
	class OptimizableAutoCompleteGraph : public g2o::SparseOptimizer{
		
	public :
		typedef g2o::BlockSolver< g2o::BlockSolverTraits<-1, -1> >  SlamBlockSolver;
		typedef g2o::LinearSolverCSparse<SlamBlockSolver::PoseMatrixType> SlamLinearSolver;
	protected:
// 		g2o::SparseOptimizer _optimizer;
		SlamLinearSolver* _linearSolver;
		SlamBlockSolver* _blockSolver;
		g2o::OptimizationAlgorithmGaussNewton* _solver;
		g2o::SE2 _sensorOffsetTransf;
		g2o::ParameterSE2Offset* _sensorOffset;
		
// 		AutoCompleteGraph* _graph;
		
	private:
		g2o::RobustKernelPseudoHuber* _huber; //Huber
		g2o::RobustKernelDCS* _dcs; //DCS
		
		bool _first;
		
	public:
		
		OptimizableAutoCompleteGraph(const g2o::SE2& sensoffset
// 				const Eigen::Vector2d& tn, 
// 				double rn,
// 				const Eigen::Vector2d& ln,
// 				const Eigen::Vector2d& pn,
// 				double rp,
// 				const Eigen::Vector2d& linkn
									) : _sensorOffsetTransf(sensoffset), _first(true){
			
			_linearSolver = new SlamLinearSolver();
			_linearSolver->setBlockOrdering(false);
			_blockSolver = new SlamBlockSolver(_linearSolver);
			_solver = new g2o::OptimizationAlgorithmGaussNewton(_blockSolver);
			_linearSolver->setBlockOrdering(false);
			this->setAlgorithm(_solver);
			
			

			// add the parameter representing the sensor offset ATTENTION was ist das ?
			_sensorOffset = new g2o::ParameterSE2Offset;
			_sensorOffset->setOffset(_sensorOffsetTransf);
			_sensorOffset->setId(0);
			this->addParameter(_sensorOffset);
			
// 			_graph = new AutoCompleteGraph(sensoffset, tn, rn, ln, pn, rp, linkn);
// 			this->addGraph(_graph);
			_huber = new g2o::RobustKernelPseudoHuber();
			_dcs = new g2o::RobustKernelDCS();
			
		};
		
		//TODO
		///@brief Does not copy pointer :)!
// 		OptimizableAutoCompleteGraph(const OptimizableAutoCompleteGraph& opt){
// 			_optimizer = opt.getoptimizer;
// 			_linearSolver = opt.getLinearSolver();
// 			_blockSolver = opt.getBlockSolver;
// 			_solver = opt.getSolver;
// 			_sensorOffsetTransf = opt.getSensorOffsetTransfo();
// 			_sensorOffset = opt.getSensorOffset();
// // 			_graph = opt.getGraph();
// 		}
		
		~OptimizableAutoCompleteGraph(){
// 			delete _graph;
// 			delete _sensorOffset;
// 			delete _blockSolver;
// 			delete _solver;
// 			delete _linearSolver;
			delete _huber;
			delete _dcs;
			
			//TODO : not sure about that
			g2o::Factory::destroy();
			g2o::OptimizationAlgorithmFactory::destroy();
			g2o::HyperGraphActionLibrary::destroy();
		}
		
// 		AutoCompleteGraph* getGraph(){return _graph;}
		g2o::ParameterSE2Offset* getSensorOffset(){return _sensorOffset;}
		SlamLinearSolver* getLinearSolver(){ return _linearSolver;}
		SlamBlockSolver* getBlockSolver(){ return _blockSolver;}
// 		const g2o::SparseOptimizer& getoptimizer(){return _optimizer;}
		g2o::OptimizationAlgorithmGaussNewton* getSolver() {return _solver;}
		const g2o::SE2& getSensorOffsetTransfo(){return _sensorOffsetTransf;}
		
		
		
		///@brief Init
// 		void init(){
// // 			setFirst();
// 			std::cout << "INIT" << std::endl;
// 			//Prepare
// // 			for (SparseOptimizer::VertexIDMap::const_iterator it = this->vertices().begin(); it != this->vertices().end(); ++it) {
// // 				OptimizableGraph::Vertex* v = static_cast<OptimizableGraph::Vertex*>(it->second);
// // 				v->setMarginalized(false);
// // 			}
// 			if (this->activeEdges().size() == 0)
// 				this->initializeOptimization();
// 			computeInitialGuess();
// 		}
		
		//TODO : attention -> make sure that the initializeOptimization does not reinitialize the state of the graph as in g2o_viewer
		void initSubset(const g2o::HyperGraph::Edge& egde){
			this->initializeOptimization();
		}
		
		//Set Marginalized to false and do initializeOptimization
		void prepare(){
			//Prepare when changing kernels
			for (SparseOptimizer::VertexIDMap::const_iterator it = vertices().begin(); it != vertices().end(); ++it) {
				OptimizableGraph::Vertex* v = static_cast<OptimizableGraph::Vertex*>(it->second);
				v->setMarginalized(false);
			}
			initializeOptimization();
		}
		
		void optimize(int iter_in = 10){
			
			setHuberKernel();
			
			int iter = g2o::SparseOptimizer::optimize(iter_in);
			if (iter > 0 && !iter){
				std::cerr << "Optimization failed, result might be invalid" << std::endl;
			}
			
		}
		
// 		///@brief Optimization process
// 		void optimize(){
// 			
// 			
// 			g2o::SparseOptimizer::initializeOptimization();
// 			g2o::SparseOptimizer::optimize(1);
// 			
// 			
// 			/*if(_first == true){
// 				std::cerr << "Preparing (no marginalization of Landmarks)" << std::endl;
// 				for (SparseOptimizer::VertexIDMap::const_iterator it = this->vertices().begin(); it != this->vertices().end(); ++it) {
// 					OptimizableGraph::Vertex* v = static_cast<OptimizableGraph::Vertex*>(it->second);
// 					v->setMarginalized(false);
// 				}
// 				_first = false;
// 			}
// 			this->initializeOptimization();
// 			//Setting no robust kernel
// 			for (SparseOptimizer::EdgeSet::const_iterator it = this->edges().begin(); it != this->edges().end(); ++it) {
// 				OptimizableGraph::Edge* e = static_cast<OptimizableGraph::Edge*>(*it);
// 				e->setRobustKernel(0);
// 			}
// 			std::cout << "Optimizing using Huber" << std::endl;
// 			//ATTENTION : Doesn't work without a kernel the pointer get lost nan with test_links. Why ?
// // 			this->setHuberKernel();
// // 			this->removeRobustKernel();
// 			int iter = g2o::SparseOptimizer::optimize(1);
// 			if (1 > 0 && !iter){
// 				std::cerr << "Optimization failed, result might be invalid" << std::endl;
// 			}
// 			std::cout << "Optimizing using DCS" << std::endl;
// // 			this->setDCSKernel();
// // 			g2o::SparseOptimizer::optimize(5);	*/		
// 		}
// 		
		
		//TODO better this
		void setHuberKernel(){
// 			for (SparseOptimizer::VertexIDMap::const_iterator it = this->vertices().begin(); it != this->vertices().end(); ++it) {
// 				OptimizableGraph::Vertex* v = static_cast<OptimizableGraph::Vertex*>(it->second);
// 				v->setMarginalized(false);
// 			}		
			
			auto idmapedges = this->edges();
			for ( auto ite = idmapedges.begin(); ite != idmapedges.end(); ++ite ){
				std::cout << "Robust Kern" << std::endl;
				OptimizableGraph::Edge* e = static_cast<OptimizableGraph::Edge*>(*ite);
				auto huber = new g2o::RobustKernelHuber();
				e->setRobustKernel(huber);
				e->robustKernel()->setDelta(1);
			}
		}
		void setDCSKernel(){
			/*for (SparseOptimizer::VertexIDMap::const_iterator it = this->vertices().begin(); it != this->vertices().end(); ++it) {
				OptimizableGraph::Vertex* v = static_cast<OptimizableGraph::Vertex*>(it->second);
				v->setMarginalized(false);
			}*/		
			
			auto idmapedges = this->edges();
			for ( auto ite = idmapedges.begin(); ite != idmapedges.end(); ++ite ){
				std::cout << "Robust Kern" << std::endl;
				OptimizableGraph::Edge* e = static_cast<OptimizableGraph::Edge*>(*ite);
				auto dcs = new g2o::RobustKernelDCS();
				e->setRobustKernel(dcs);
				e->robustKernel()->setDelta(1);
			}
		}
		void removeRobustKernel(){
// 			for (SparseOptimizer::VertexIDMap::const_iterator it = this->vertices().begin(); it != this->vertices().end(); ++it) {
// 				OptimizableGraph::Vertex* v = static_cast<OptimizableGraph::Vertex*>(it->second);
// 				v->setMarginalized(false);
// 			}		
			
			//Setting robust kernel
			for (SparseOptimizer::EdgeSet::const_iterator it = edges().begin(); it != edges().end(); ++it) {
				OptimizableGraph::Edge* e = static_cast<OptimizableGraph::Edge*>(*it);
				e->setRobustKernel(0);
			}
			
		}
		
	
		
		void setFirst(){
// 			auto firstRobotPose = this->vertex(0);
// 			firstRobotPose->setFixed(true);
			bool gaugeFreedo = gaugeFreedom();
			g2o::OptimizableGraph::Vertex* gauge = findGauge();
			if (gaugeFreedo) {
				if (! gauge) {
				std::cerr <<  "cannot find a vertex to fix in this thing" << std::endl;
				return;
				} else {
				std::cerr << "graph is fixed by node " << gauge->id() << std::endl;
				gauge->setFixed(true);
				}
			} else {
				std::cerr << "graph is fixed by priors or nodes are already fixed" << std::endl;
			}
		}
		
		private:
		
		void setRobustKernelAllEdges(g2o::RobustKernel* ptr = NULL, double width = 1){
			
			for (SparseOptimizer::VertexIDMap::const_iterator it = this->vertices().begin(); it != this->vertices().end(); ++it) {
				OptimizableGraph::Vertex* v = static_cast<OptimizableGraph::Vertex*>(it->second);
				v->setMarginalized(false);
			}		
			
			auto idmapedges = this->edges();
			if(ptr != NULL){
				for ( auto ite = idmapedges.begin(); ite != idmapedges.end(); ++ite ){
					std::cout << "Robust Kern" << std::endl;
					OptimizableGraph::Edge* e = static_cast<OptimizableGraph::Edge*>(*ite);
					e->setRobustKernel(ptr);
					e->robustKernel()->setDelta(width);
				}
			}
			
			//Same as ficing it to null to remove kernel
// 			else{
// 				for ( auto ite = idmapedges.begin(); ite != idmapedges.end(); ++ite ){
// 					std::cout << "Non robust Kern" << std::endl;
// 					OptimizableGraph::Edge* e = static_cast<OptimizableGraph::Edge*>(*ite);
// 					e->setRobustKernel(0);
// 				}
// 			}
			
// 			double huberWidth = 1;  
			//odometry edges are those whose node ids differ by 1
			
// 			bool onlyLoop = cbOnlyLoop->isChecked();

			/*for (SparseOptimizer::EdgeSet::const_iterator it = optimizer->edges().begin(); it != optimizer->edges().end(); ++it) {
				OptimizableGraph::Edge* e = static_cast<OptimizableGraph::Edge*>(*it);
// 				if (onlyLoop) {
// 					if (e->vertices().size() >= 2 && std::abs(e->vertex(0)->id() - e->vertex(1)->id()) != 1) {
// 					e->setRobustKernel(creator->construct());
// 					e->robustKernel()->setDelta(huberWidth);
// 					}
// 				} else {
					e->setRobustKernel(creator->construct());
					e->robustKernel()->setDelta(huberWidth);
// 				}
			}   */ 
			
		}
		
	};
}
}

#endif