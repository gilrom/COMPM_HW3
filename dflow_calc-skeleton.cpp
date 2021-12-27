/* 046267 Computer Architecture - Winter 20/21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"

Node::Node() :
    op(),
    op_num(0),
    latency(0),
    father_one(nullptr),
    father_two(nullptr),
    IsDep(false)
{}

Node::Node(InstInfo op_recieved, unsigned int op_num, unsigned int latency) :
    op_num(0),
    latency(0),
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
    delete ctx;
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

ProgDepTree::ProgDepTree(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts)
 : ops(nullptr), numOfInsts(numOfInsts){
    ops = new Node* [numOfInsts];

    for (int i=0 ; i<numOfInsts ; i++)
    {
        ops[i] = new Node(progTrace[i], i, opsLatency[progTrace[i].opcode]);
    }

    for(int i=0; i<REGS_NUM; i++)
    {
        reg_deps[i] = nullptr;
    }

    for (int i=0; i<numOfInsts; i++)
    {
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
    for (int i=0; i < numOfInsts; i++){
        delete ops[i];
    }
    delete[] ops;
}

int ProgDepTree::getInstDeps(unsigned int theInst, int *src1DepInst, int *src2DepInst)
{

    int deps = (ops[theInst]->father_one != nullptr) + (ops[theInst]->father_two != nullptr);

    switch (deps)
    {
    case 2:
        *src1DepInst = ops[theInst]->father_one->op_num;
        *src2DepInst = ops[theInst]->father_two->op_num;
        break;

    case 1:
        if(ops[theInst]->father_one != nullptr)
        {
            *src1DepInst = ops[theInst]->father_one->op_num;
        }
        else
        {
            *src1DepInst = ops[theInst]->father_two->op_num;
        }
        break;

    case 0:
        return -1;
        break;
    
    default:
        return -1;
        break;
    }
}

int ProgDepTree::getInstDepth(unsigned int theInst)
{
    return (maxLatencyFromOp(ops[theInst]) - ops[theInst]->latency);
}

int ProgDepTree::getProgDepth()
{
    int max = 0;
    for (int i = 0 ; i < numOfInsts ; i++)
    {
        if(!ops[i]->IsDep)
        {
            max = Maximum(max, maxLatencyFromOp(ops[i]));
        }
    }
    return max;
}