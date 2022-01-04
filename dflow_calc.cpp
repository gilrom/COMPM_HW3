/* 046267 Computer Architecture - Winter 20/21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"

#define REGS_NUM 32

//Defines a node in dependencies tree
class Node
{
    public:
        InstInfo op;
        unsigned int op_num;
        unsigned int latency;
        Node* father_one;
        Node* father_two;
        bool IsDep;

        Node(/* args */);
        Node(InstInfo op_recieved, unsigned int op_num, unsigned int latency);
};

//data starcture for dependencies tree
class ProgDepTree{
    private:
        Node** ops; //operations nodes
        Node* reg_deps[REGS_NUM]; // an array to save the last operation writed to register
        unsigned int numOfInsts;
        
    public:
        ProgDepTree(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts);
        ~ProgDepTree();
        int getProgDepth();
        int getInstDeps(unsigned int theInst, int *src1DepInst, int *src2DepInst);
        int getInstDepth(unsigned int theInst);
};

//Node empty ctor
Node::Node() :
    op(),
    op_num(0),
    latency(0),
    father_one(nullptr),
    father_two(nullptr),
    IsDep(false)
{}

//ctor
Node::Node(InstInfo op_recieved, unsigned int op_num, unsigned int latency) :
    op_num(op_num),
    latency(latency),
    father_one(nullptr),
    father_two(nullptr),
    IsDep(false)

    {
        op = op_recieved;
    }

inline unsigned int Maximum(unsigned int a, unsigned int b)
{
    if(a>b) return a;
    else return b;
}

//recursive function to iterate over the tree and find the max latency rout for specific node
unsigned int maxLatencyFromOp(Node* op)
{
    if(op == nullptr)
    {
        return 0;
    }
    if(op->father_one == nullptr && op->father_two == nullptr)
    {
        return op->latency;
    }

    return op->latency + Maximum(maxLatencyFromOp(op->father_one),
                   maxLatencyFromOp(op->father_two));
}

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    return new ProgDepTree(opsLatency, progTrace, numOfInsts);
    //return PROG_CTX_NULL;
}

void freeProgCtx(ProgCtx ctx) {
    ProgDepTree* obj = (ProgDepTree*)ctx;
    delete obj;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    ProgDepTree* obj = (ProgDepTree*)ctx;
    return obj->getInstDepth(theInst);
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    ProgDepTree* obj = (ProgDepTree*)ctx;
    return obj->getInstDeps(theInst, src1DepInst, src2DepInst);
}

int getProgDepth(ProgCtx ctx) {
    ProgDepTree* obj = (ProgDepTree*)ctx;
    return obj->getProgDepth();
}

//ctor for tree
ProgDepTree::ProgDepTree(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts)
 : ops(nullptr), numOfInsts(numOfInsts){
    ops = new Node* [numOfInsts];

    for (unsigned int i=0 ; i<numOfInsts ; i++)
    {
        ops[i] = new Node(progTrace[i], i, opsLatency[progTrace[i].opcode]);
    }

    for(int i=0; i<REGS_NUM; i++)
    {
        reg_deps[i] = nullptr;
    }

    for (unsigned int i=0; i<numOfInsts; i++)
    {
        //if op reads from previous writen register
        if(reg_deps[ops[i]->op.src1Idx] != nullptr)
        {
            ops[i]->father_one = reg_deps[ops[i]->op.src1Idx];
            reg_deps[ops[i]->op.src1Idx]->IsDep = true;
        }

        if(reg_deps[ops[i]->op.src2Idx] != nullptr)
        {
            ops[i]->father_two = reg_deps[ops[i]->op.src2Idx];
            reg_deps[ops[i]->op.src2Idx]->IsDep = true;
        }

        reg_deps[ops[i]->op.dstIdx] = ops[i];
    }
}

ProgDepTree::~ProgDepTree()
{
    for (unsigned int i=0; i < numOfInsts; i++){
        delete ops[i];
    }
    delete[] ops;
}

int ProgDepTree::getInstDeps(unsigned int theInst, int *src1DepInst, int *src2DepInst)
{
    *src1DepInst = ops[theInst]->father_one != nullptr ? ops[theInst]->father_one->op_num : -1;
    *src2DepInst = ops[theInst]->father_two != nullptr ? ops[theInst]->father_two->op_num : -1;
    return 0;
}

int ProgDepTree::getInstDepth(unsigned int theInst)
{
    return (maxLatencyFromOp(ops[theInst]) - ops[theInst]->latency);
}

int ProgDepTree::getProgDepth()
{
    int max = 0;
    for (unsigned int i = 0 ; i < numOfInsts ; i++)
    {
        if(!ops[i]->IsDep)
        {
            max = Maximum(max, maxLatencyFromOp(ops[i]));
        }
    }
    return max;
}