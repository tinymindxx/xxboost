#ifndef _FASTRGF_DISCRETIZATION_H
#define _FASTRGF_DISCRETIZATION_H

typedef const float d;

#include "data.h"

namespace rgf {

    class FeatureDiscretizationDense {
    public:
        UniqueArray<float> boundaries;

        class TrainParam : public ParameterParser {
        public:
            ParamValue<double> min_bucket_weights;
            ParamValue<int> max_buckets;
            ParamValue<double> lamL2;

            TrainParam(string prefix = "disc_dense.") {
                min_bucket_weights.insert(prefix + "min_bucket_weights",
                                          5,
                                          "minimum number of effective samples for each discretization value",
                                          this
                );

                max_buckets.insert(prefix + "max_buckets",
                                   65000,
                                   "maximum number of discretized values",
                                   this);
                lamL2.insert(prefix + "lamL2",
                             2,
                             "L2 regulation parameter",
                             this);

            }
        };

        FeatureDiscretizationDense() {}

        size_t size() {
            return boundaries.size() + 1;
        }

        void set(UniqueArray<float> &boundaries0) {
            boundaries = std::move(boundaries0);
        }

        pair<float, float> operator[](const int v) {
            float low, high;
            if (v <= 0) {
                low = 1e-10;
            } else {
                low = boundaries[v - 1];
            }

            if (v + 1 >= boundaries.size()) {
                high = 1e20;
            } else {
                high = boundaries[v];
            }
            return pair<float, float>(low, high);
        }

        int apply(float x);

        void clear() {
            boundaries.clear();
        }

        template<typename i_t>
        void train(DataSet<float, i_t, float> &ds, int j, TrainParam &tr);

        void read(istream &is);

        void write(ostream &os);
    };

    template<typename feat_t, typename id_t, typename disc_t>
    class FeatureDiscretizationSparse {
    public:
        unordered_map<feat_t, id_t> feat2id;
        UniqueArray<feat_t> id2feat;

        UniqueArray<FeatureDiscretizationDense> boundary_arr;

        class TrainParam : public ParameterParser {
        public:
            ParamValue<double> min_bucket_weights;


            ParamValue<int> max_buckets;


            ParamValue<int> max_features;


            ParamValue<int> min_occurrences;


            ParamValue <string> missing_type;


            ParamValue<float> lamL2;


            TrainParam(string prefix = "disc_sparse.") {
                min_bucket_weights.insert(prefix + "min_bucket_weights",
                                          5,
                                          "minimum number of effective samples for each discretized value",
                                          this);
                max_buckets.insert(prefix + "max_buckets",
                                   200,
                                   "maximum number of discretized values",
                                   this);
                max_features.insert(prefix + "max_features",
                                    80000,
                                    "maximum number of selected features",
                                    this);
                min_occurrences.insert(prefix + "min_occrrences",
                                       5,
                                       "minimum number of occurrences for a feature to be selected",
                                       this);
                missing_type.insert(prefix + "missing_type",
                                    "MIN",
                                    "sparse missing feature type: current only handle MIN",
                                    this);
                lamL2.insert(prefix + "lamL2", 2,
                             "L2 regularization parameter", this);

            }
        };

        FeatureDiscretizationSparse() {}

        size_t size() {
            return id2feat.size();
        }

        FeatureDiscretizationDense *operator[](const int id) {
            if (id >= 0 && id < boundary_arr.size()) {
                return &boundary_arr[id];
            }
            return nullptr;
        }

        void clear() {
            assert(feat2id.size() == size() && id2feat.size() == size() && boundary_arr.size() == size());
            feat2id.clear();
            id2feat.clear();
            boundary_arr.clear();
        }

        void train(DataSet<float, feat_t, float> &ds, int j, TrainParam &tr, int nthreads, int verbose);

        void write(ostream &os);

        void read(istream &is);
    };


    template<typename src_i_t, typename dest_d_t, typename dest_i_t, typename dest_v_t>
    class DataDiscretization {
        vector<int> _offset;

        void offset_init();


        enum convert_t {
            MIX = 0,
            DENSE = 1,
            SPARSE = 2
        } covert_type = convert_t::MIX;

    public:
        void set_covert(string str) {
            if (str.compare("MIX") == 0) {
                covert_type = MIX;
            } else if (str.compare("DENSE") == 0) {
                covert_type = DENSE;
            } else if (str.compare("SPARSE") == 0) {
                covert_type = SPARSE;
            } else {
                cerr << "invalid data discretization type " << str << endl;
                exit(-1);
            }
        }

        UniqueArray<FeatureDiscretizationDense> disc_dense;
        UniqueArray<FeatureDiscretizationSparse<src_i_t, dest_i_t, dest_v_t> > disc_sparse;

        void train(DataSet<float, src_i_t, float> &ds, FeatureDiscretizationDense::TrainParam &tr_dense,
                   typename FeatureDiscretizationSparse<src_i_t, dest_i_t, dest_v_t>::TrainParam &tr_sparse,
                   int nthread, int verbose);

//        bool apply(DataSet<float, src_i_t, float> &src, DataPoint<dest_d_t, dest_i_t, dest_v_t> &dest,
//                   bool is_sorted = true);
//
//        bool apply(DataSet<float, src_i_t, float> &src, DataPoint<dest_d_t, dest_i_t, dest_v_t> &dest,
//                   int nthreads = 0);
//
//        void read(istream &is);
//
//        void write(ostream &os);

    };

    using FeatureDiscretizationSparseInt=FeatureDiscretizationSparse<src_index_t, int, int>;
    using DataDiscretizationInt = DataDiscretization<src_index_t, int, int, int>;
}


#endif //FASTRGF_DISCRITIZATION_H
