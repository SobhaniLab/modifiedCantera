#include "ReactorNet.h"
#include "Integrator.h"

namespace CanteraZeroD {

    ReactorNet::ReactorNet() : FuncEval(), m_nr(0), m_nreactors(0),
                               m_integ(0), m_time(0.0), m_init(false), 
                               m_nv(0), m_rtol(1.0e-9), m_rtolsens(1.0e-4), 
                               m_atols(1.0e-15), m_atolsens(1.0e-4),
                               m_maxstep(-1.0),
                               m_verbose(false), m_ntotpar(0)
    {
#ifdef DEBUG_MODE
        m_verbose = true;
#endif
        m_integ = newIntegrator("CVODE");// CVodeInt;

        // use backward differencing, with a full Jacobian computed
        // numerically, and use a Newton linear iterator

        m_integ->setMethod(BDF_Method);
        m_integ->setProblemType(DENSE + NOJAC);
        m_integ->setIterator(Newton_Iter);        
    }

    void ReactorNet::initialize(doublereal t0) {
        int n, nv;
        char buf[100];
        m_nv = 0;
        m_reactors.clear();
        m_nreactors = 0;
        if (m_verbose) {
            writelog("Initializing reactor network.\n");
        }
        for (n = 0; n < m_nr; n++) {
            if (m_r[n]->type() >= ReactorType) {
                m_r[n]->initialize(t0);
                Reactor* r = (Reactor*)m_r[n];
                m_reactors.push_back(r);
                nv = r->neq();
                m_size.push_back(nv);
                m_nparams.push_back(r->nSensParams());
                m_ntotpar += r->nSensParams();
                m_nv += nv;
                m_nreactors++;
                if (m_verbose) {
                    sprintf(buf,"Reactor %d: %d variables.\n",n,nv);
                    writelog(buf);
                    sprintf(buf,"            %d sensitivity params.\n",
                        r->nSensParams());
                    writelog(buf);
                }
                if (m_r[n]->type() == FlowReactorType && m_nr > 1) {
                    throw CanteraError("ReactorNet::initialize",
                        "FlowReactors must be used alone.");
                }
            }
        }

        m_atol.resize(neq());
        fill(m_atol.begin(), m_atol.end(), m_atols);
        m_integ->setTolerances(m_rtol, neq(), m_atol.begin());
        m_integ->setSensitivityTolerances(m_rtolsens, m_atolsens);
        m_integ->setMaxStepSize(m_maxstep);
        if (m_verbose) {
            sprintf(buf, "Number of equations: %d\n", neq());
            writelog(buf);
            sprintf(buf, "Maximum time step:   %14.6g\n", m_maxstep);
            writelog(buf);
        }
        m_integ->initialize(t0, *this);
        m_init = true;
    }

    void ReactorNet::advance(doublereal time) {
        if (!m_init) {
            if (m_maxstep < 0.0)
                m_maxstep = time - m_time;
            initialize();
        }
        m_integ->integrate(time);
        m_time = time;
        updateState(m_integ->solution());
    }

    double ReactorNet::step(doublereal time) {
        if (!m_init) {
            if (m_maxstep < 0.0)
                m_maxstep = time - m_time;
            initialize();
        }
        m_time = m_integ->step(time);
        updateState(m_integ->solution());
        return m_time;
    }

//     void ReactorNet::addSensitivityParam(int n, int stype, int i) {
//         m_reactors[n]->addSensitivityParam(int stype, int i);
//         m_sensreactor.push_back(n);
//         m_nSenseParams++;
//     }

//     void ReactorNet::setParameters(int np, double* p) {
//         int n, nr;
//         for (n = 0; n < np; n++) {
//             if (n < m_nSenseParams) {
//                 nr = m_sensreactor[n];
//                 m_reactors[nr]->setParameter(n, p[n]);
//             }
//         }
//     }
        
    void ReactorNet::eval(doublereal t, doublereal* y, 
        doublereal* ydot, doublereal* p) {
        int n;
        int start = 0;
        int pstart = 0;
        // use a try... catch block, since exceptions are not passed
        // through CVODE, since it is C code
        try {
            updateState(y);
            for (n = 0; n < m_nreactors; n++) {
                m_reactors[n]->evalEqs(t, y + start, 
                    ydot + start, p + pstart);
                start += m_size[n];
                pstart += m_nparams[n];
            }
        }
        catch (...) {
            showErrors();
            error("Terminating execution.");
        }
    }

    void ReactorNet::updateState(doublereal* y) {
        int n;
        int start = 0;
        for (n = 0; n < m_nreactors; n++) {
            m_reactors[n]->updateState(y + start);
            start += m_size[n];
        }
    }

    void ReactorNet::getInitialConditions(doublereal t0, 
        size_t leny, doublereal* y) {
        int n;
        int start = 0;
        for (n = 0; n < m_nreactors; n++) {
            m_reactors[n]->getInitialConditions(t0, m_size[n], y + start);
            start += m_size[n];
        }
    }

    int ReactorNet::globalComponentIndex(string species, int reactor) {
        int start = 0;
        int n;
        for (n = 0; n < reactor; n++) start += m_size[n];
        return start + m_reactors[n]->componentIndex(species);
    }

}

