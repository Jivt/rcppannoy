// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
//  RcppAnnoy -- Rcpp bindings to Annoy library for Approximate Nearest Neighbours
//
//  Copyright (C) 2014 - 2016  Dirk Eddelbuettel
//
//  This file is part of RcppAnnoy
//
//  RcppAnnoy is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  RcppAnnoy is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with RcppAnnoy.  If not, see <http://www.gnu.org/licenses/>.

// simple C++ modules to wrap to two templated classes from Annoy
//
// uses annoylib.h (from Annoy) and provides R access via Rcpp
//
// Dirk Eddelbuettel, Nov 2014


#include <Rcpp.h>

#if defined(__MINGW32__)
#undef Realloc
#undef Free
#endif

// define R's REprintf as the 'local' error print method for Annoy
#define __ERROR_PRINTER_OVERRIDE__  REprintf

#include "annoylib.h"
#include "kissrandom.h"

template< typename S, typename T, typename Distance, typename Random >
class Annoy
{
protected:
    AnnoyIndex<S, T, Distance, Random> *ptr;
    unsigned int vectorsz;
public:
    Annoy(int n) : vectorsz(n) {
        ptr = new AnnoyIndex<S, T, Distance, Random>(n);
    }
    ~Annoy() { if (ptr != NULL) delete ptr; }
    void addItem(int32_t item, Rcpp::NumericVector dv) {
        if (item < 0) Rcpp::stop("Inadmissible item value %d", item);
        std::vector<float> fv(dv.size());
        std::copy(dv.begin(), dv.end(), fv.begin());
        ptr->add_item(item, &fv[0]);
    }
    void   callBuild(int n)               { ptr->build(n);                  }
    void   callSave(std::string filename) { ptr->save(filename.c_str());    }
    void   callLoad(std::string filename) { ptr->load(filename.c_str());    }
    void   callUnload()                   { ptr->unload();                  }
    int    getNItems()                    { return ptr->get_n_items();      }
    double getDistance(int i, int j)      { return ptr->get_distance(i, j); }
    void   verbose(bool v)                { ptr->verbose(v);                }

    std::vector<int32_t> getNNsByItem(int32_t item, size_t n) {
        std::vector<int32_t> result;
        ptr->get_nns_by_item(item, n, -1, &result, NULL);
        return result;
    }

    Rcpp::List getNNsByItemList(S item, size_t n,
                                size_t search_k, bool include_distances) {
        if (include_distances) {
            std::vector<S> result;
            std::vector<T> distances;
            ptr->get_nns_by_item(item, n, search_k, &result, &distances);
            return Rcpp::List::create(Rcpp::Named("item")     = result,
                                      Rcpp::Named("distance") = distances);
        } else {
            std::vector<S> result;
            ptr->get_nns_by_item(item, n, search_k, &result, NULL);
            return Rcpp::List::create(Rcpp::Named("item") = result);
        }
    }

    std::vector<int32_t> getNNsByVector(std::vector<double> dv, size_t n) {
        std::vector<float> fv(dv.size());
        std::copy(dv.begin(), dv.end(), fv.begin());
        std::vector<int32_t> result;
        ptr->get_nns_by_vector(&fv[0], n, -1, &result, NULL);
        return result;
    }
    
    Rcpp::List getNNsByVectorList(std::vector<T> fv, size_t n,
                                  size_t search_k, bool include_distances) {
        if (fv.size() != vectorsz) {
            Rcpp::stop("fv.size() != vector_size");
        }
        if (include_distances) {
            std::vector<S> result;
            std::vector<T> distances;
            ptr->get_nns_by_vector(&fv[0], n, search_k, &result, &distances);
            return Rcpp::List::create(
                Rcpp::Named("item") = result,
                Rcpp::Named("distance") = distances);
        } else {
            std::vector<S> result;
            ptr->get_nns_by_vector(&fv[0], n, search_k, &result, NULL);
            return Rcpp::List::create(Rcpp::Named("item") = result);
        }
    }
    
    std::vector<double> getItemsVector(int32_t item) {
        std::vector<float> fv;
        ptr->get_item(item, &fv);
        std::vector<double> dv(fv.size());
        std::copy(fv.begin(), fv.end(), dv.begin());
        return dv;
    }

};

// this breaks Rcpp Modules as we can not have nested (ie two-level) templates :-/
typedef Annoy<int32_t, float, Angular, Kiss64Random>   AnnoyAngular;
typedef Annoy<int32_t, float, Euclidean, Kiss64Random> AnnoyEuclidean;

// this does too
//typedef Angular<int32_t, float, Kiss64Random>     AnnoyAngularDist;
//typedef Euclidean<int32_t, float, Kiss64Random>   AnnoyEucledianDist;
//typedef Annoy<int32_t, float, AnnoyAngularDist>   AnnoyAngular;
//typedef Annoy<int32_t, float, AnnoyEucledianDist> AnnoyEuclidean;
/*
class AnnoyBase {
protected:
    AnnoyIndexInterface<int32_t, float> *ptr;
public:
    AnnoyBase(int n) { ptr = nullptr; }
    void addItem(int32_t item, Rcpp::NumericVector dv) {
        std::vector<float> fv(dv.size());
        std::copy(dv.begin(), dv.end(), fv.begin());
        ptr->add_item(item, &fv[0]);
    }
    void   callBuild(int n)               { ptr->build(n);                  }
    void   callSave(std::string filename) { ptr->save(filename.c_str());    }
    void   callLoad(std::string filename) { ptr->load(filename.c_str());    }
    void   callUnload()                   { ptr->unload();                  }
    int    getNItems()                    { return ptr->get_n_items();      }
    double getDistance(int i, int j)      { return ptr->get_distance(i, j); }
    void   verbose(bool v)                { ptr->verbose(v);                }

    std::vector<int32_t> getNNsByItem(int32_t item, size_t n) {
        std::vector<int32_t> result;
        ptr->get_nns_by_item(item, n, -1, &result, NULL);
        return result;
    }

    std::vector<int32_t> getNNsByVector(std::vector<double> dv, size_t n) {
        std::vector<float> fv(dv.size());
        std::copy(dv.begin(), dv.end(), fv.begin());
        std::vector<int32_t> result;
        ptr->get_nns_by_vector(&fv[0], n, -1, &result, NULL);
        return result;
    }

    std::vector<double> getItemsVector(int32_t item) {
        std::vector<float> fv;
        ptr->get_item(item, &fv);
        std::vector<double> dv(fv.size());
        std::copy(fv.begin(), fv.end(), dv.begin());
        return dv;
    }

};

// we need to repeat the actual functions here to provide Rcpp Modules with access points
// standard C++ inheritance does not work for the Modules interface
class AnnoyAngular : public AnnoyBase {
public:
    AnnoyAngular(int n) : AnnoyBase(n) {
        ptr = new AnnoyIndex<int32_t, float, Angular, Kiss64Random>(n);
    }
    inline void addItem(int32_t item, Rcpp::NumericVector dv) { AnnoyBase::addItem(item, dv);  }
    inline void callBuild(int n)                              { AnnoyBase::callBuild(n);       }
    inline void callSave(std::string filename)                { AnnoyBase::callSave(filename); }
    inline void callLoad(std::string filename)                { AnnoyBase::callLoad(filename); }
    inline void callUnload()                                  { AnnoyBase::callUnload();       }
    inline int  getNItems()                                   { return AnnoyBase::getNItems();       }
    inline double getDistance(int i, int j)                   { return AnnoyBase::getDistance(i, j); }
    inline void verbose(bool v)                               { AnnoyBase::verbose(v);               }
    inline std::vector<int32_t> getNNsByItem(int32_t item, size_t n)
                                                              { return AnnoyBase::getNNsByItem(item, n); }
    inline std::vector<int32_t> getNNsByVector(std::vector<double> dv, size_t n)
                                                              { return AnnoyBase::getNNsByVector(dv, n); }
    inline std::vector<double> getItemsVector(int32_t item)   { return AnnoyBase::getItemsVector(item);  }
};

class AnnoyEuclidean : public AnnoyBase {
public:
    AnnoyEuclidean(int n) : AnnoyBase(n) {
        ptr = new AnnoyIndex<int32_t, float, Euclidean, Kiss64Random>(n);
    }
    inline void addItem(int32_t item, Rcpp::NumericVector dv) { AnnoyBase::addItem(item, dv);  }
    inline void callBuild(int n)                              { AnnoyBase::callBuild(n);       }
    inline void callSave(std::string filename)                { AnnoyBase::callSave(filename); }
    inline void callLoad(std::string filename)                { AnnoyBase::callLoad(filename); }
    inline void callUnload()                                  { AnnoyBase::callUnload();       }
    inline int  getNItems()                                   { return AnnoyBase::getNItems();       }
    inline double getDistance(int i, int j)                   { return AnnoyBase::getDistance(i, j); }
    inline void verbose(bool v)                               { AnnoyBase::verbose(v);               }
    inline std::vector<int32_t> getNNsByItem(int32_t item, size_t n)
                                                              { return AnnoyBase::getNNsByItem(item, n); }
    inline std::vector<int32_t> getNNsByVector(std::vector<double> dv, size_t n)
                                                              { return AnnoyBase::getNNsByVector(dv, n); }
    inline std::vector<double> getItemsVector(int32_t item)   { return AnnoyBase::getItemsVector(item);  }
};
*/



//RCPP_EXPOSED_CLASS_NODECL(AnnoyAngular)
RCPP_MODULE(AnnoyAngular) {
    Rcpp::class_<AnnoyAngular>("AnnoyAngular")

        .constructor<int32_t>("constructor with integer count")

        .method("addItem",        &AnnoyAngular::addItem,         "add item")
        .method("build",          &AnnoyAngular::callBuild,       "build an index")
        .method("save",           &AnnoyAngular::callSave,        "save index to file")
        .method("load",           &AnnoyAngular::callLoad,        "load index from file")
        .method("unload",         &AnnoyAngular::callUnload,      "unload index")
        .method("getDistance",    &AnnoyAngular::getDistance,     "get distance between i and j")
        .method("getNNsByItem",   &AnnoyAngular::getNNsByItem,
                "retrieve Nearest Neigbours given item")
        .method("getNNsByItemList",  &AnnoyAngular::getNNsByItemList,
                "retrieve Nearest Neigbours given item")
        .method("getNNsByVector", &AnnoyAngular::getNNsByVector,
                "retrieve Nearest Neigbours given vector")
        .method("getNNsByVectorList",  &AnnoyAngular::getNNsByVectorList,
                "retrieve Nearest Neigbours given vector")
        .method("getItemsVector", &AnnoyAngular::getItemsVector,  "retrieve item vector")
        .method("getNItems",      &AnnoyAngular::getNItems,       "get N items")
        .method("setVerbose",     &AnnoyAngular::verbose,         "set verbose")
        ;
}

RCPP_EXPOSED_CLASS_NODECL(AnnoyEuclidean)
RCPP_MODULE(AnnoyEuclidean) {
    Rcpp::class_<AnnoyEuclidean>("AnnoyEuclidean")

        .constructor<int32_t>("constructor with integer count")

        .method("addItem",        &AnnoyEuclidean::addItem,        "add item")
        .method("build",          &AnnoyEuclidean::callBuild,      "build an index")
        .method("save",           &AnnoyEuclidean::callSave,       "save index to file")
        .method("load",           &AnnoyEuclidean::callLoad,       "load index from file")
        .method("unload",         &AnnoyEuclidean::callUnload,     "unload index")
        .method("getDistance",    &AnnoyEuclidean::getDistance,    "get distance between i and j")
        .method("getNNsByItem",   &AnnoyEuclidean::getNNsByItem,
                "retrieve Nearest Neigbours given item")
        .method("getNNsByItemList",  &AnnoyEuclidean::getNNsByItemList,
                "retrieve Nearest Neigbours given item")
        .method("getNNsByVector", &AnnoyEuclidean::getNNsByVector,
                "retrieve Nearest Neigbours given vector")
        .method("getNNsByVectorList",&AnnoyEuclidean::getNNsByVectorList,
                "retrieve Nearest Neigbours given vector")
        .method("getItemsVector", &AnnoyEuclidean::getItemsVector, "retrieve item vector")
        .method("getNItems",      &AnnoyEuclidean::getNItems,      "get N items")
        .method("setVerbose",     &AnnoyEuclidean::verbose,        "set verbose")
        ;
}
