#include <lemon/lgf_writer.h>
#include <lemon/capacity_scaling.h>
#include "offline.h"

using namespace lemon;

int main(int argc, char* argv[]) {

    std::string path(argv[1]);
    uint64_t cacheSize(atoll(argv[2]));

    // parse trace file
    std::vector<std::tuple<uint64_t,uint64_t,bool> > trace;
    uint64_t totalUniqC = parseTraceFile(trace, path);
    uint64_t totalReqc = trace.size();
    std::cout << "scanned trace n=" << totalReqc << " m=" << totalUniqC << std::endl;

    // create mcf instance
    SmartDigraph g; // mcf graph
    SmartDigraph::ArcMap<int64_t> cap(g); // mcf capacities
    SmartDigraph::ArcMap<double> cost(g); // mcf costs
    SmartDigraph::NodeMap<int64_t> supplies(g); // mcf demands/supplies
    createMCF(g, trace, cacheSize, cap, cost, supplies);
    trace.clear();
    
    std::cout << "created graph with ";
    uint64_t nodes=0, vertices=0;
    for (SmartDigraph::NodeIt n(g); n!=INVALID; ++n) ++nodes;
    std::cout << nodes << " nodes ";
    for (SmartDigraph::ArcIt a(g); a != INVALID; ++a) ++vertices;
    std::cout << vertices << " arcs " << std::endl;

    // solve the mcf instance
    CapacityScaling<SmartDigraph, int64_t, double, CapacityScalingDefaultTraits<SmartDigraph, int64_t, double>> cs(g);
    cs.upperMap(cap).costMap(cost).supplyMap(supplies);
    ProblemType res = cs.run();
    switch(res) {
        case INFEASIBLE:
            std::cout << "infeasible mcf" << std::endl;
            break;
        case UNBOUNDED:
            std::cout << "unbounded mcf" << std::endl;
            break;
        case OPTIMAL:
            std::cout << "Total cost: " << cs.totalCost<double>() << " Total Reqs " << totalReqc << " OHR " << 1.0-(static_cast<double>(cs.totalCost<double>())+totalUniqC)/totalReqc << std::endl;
            SmartDigraph::ArcMap<uint64_t> flow(g);
            cs.flowMap(flow);
            break;
    }
    // todo: check other capacity scaling factors, factor=1 disables scaling

    

    return 0;
}
