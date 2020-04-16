#include "warp.hpp"

DEFINE_string(dir, "./", "output directory");
DEFINE_string(prefix, "./prefix", "prefix of result files");
DEFINE_int32(niter, 10, "number of iterations");
DEFINE_int32(k, 1000, "number of topics");
DEFINE_double(alpha, 50, "sum of alpha");
DEFINE_double(beta, 0.01, "beta");
DEFINE_int32(mh, 1, "number of Metropolis-Hastings steps");
DEFINE_int32(ntop, 20, "num top words per each topic");
DEFINE_string(bin, "", "binary file");
DEFINE_string(model, "", "model file");
DEFINE_string(info, "", "info");
DEFINE_string(vocab, "", "vocabulary file");
DEFINE_string(topics, "", "topic assignment file");
DEFINE_string(z, "", "Z file name");
DEFINE_string(dtdist, "", "Doc Topic Dist file name");
DEFINE_bool(estimate, false, "estimate model parameters");
DEFINE_bool(inference, false, "inference latent topic assignments");
DEFINE_bool(writeinfo, true, "write info");
DEFINE_bool(dumpmodel, false, "dump model");
DEFINE_bool(dumpz, true, "dump Z");
DEFINE_int32(perplexity, -1, "Interval to evaluate perplexity. -1 for don't evaluate.");

int run_wlda(int argc, char** argv)
{
    // resets flags from previous executions
    FLAGS_bin = "";
    FLAGS_model = "";
    FLAGS_info = "";
    FLAGS_vocab = "";
    FLAGS_topics  = "";
    FLAGS_z = "";
    FLAGS_dtdist = "";
    FLAGS_dir = "";

    gflags::SetUsageMessage("Usage : ./warplda [ flags... ]");
	gflags::ParseCommandLineFlags(&argc, &argv, true);

    FLAGS_dir = FLAGS_dir + "/";

    if ((FLAGS_inference || FLAGS_estimate) == false)
        FLAGS_estimate = true;
    if (!FLAGS_z.empty())
        FLAGS_dumpz = true;

    //input
    SetIfEmpty(FLAGS_bin, FLAGS_prefix + ".bin");
    SetIfEmpty(FLAGS_vocab, FLAGS_prefix + ".vocab");

    // not being used?
    SetIfEmpty(FLAGS_topics, FLAGS_dir + FLAGS_prefix + ".topics");

    // output
    SetIfEmpty(FLAGS_model, FLAGS_dir + FLAGS_prefix + ".model");
    SetIfEmpty(FLAGS_info, FLAGS_dir + "top_words");
    SetIfEmpty(FLAGS_dtdist, FLAGS_dir + "wdistribution.txt");


    LDA *lda = new WarpLDA<1>();
    lda->loadBinary(FLAGS_bin);
    if (FLAGS_estimate)
    {
        lda->estimate(FLAGS_k, FLAGS_alpha / FLAGS_k, FLAGS_beta, FLAGS_niter, FLAGS_perplexity);
        if (FLAGS_dumpmodel)
        {
//            std::cout << "Dump model " << FLAGS_model << std::endl;
            lda->storeModel(FLAGS_model);
        }
        if (FLAGS_writeinfo)
        {
//            std::cout << "Write Info " << FLAGS_info << " ntop " << FLAGS_ntop << std::endl;
            lda->writeInfo(FLAGS_vocab, FLAGS_info, FLAGS_ntop);
        }
        if (FLAGS_dumpz)
        {
            SetIfEmpty(FLAGS_z, FLAGS_prefix + ".z.estimate");
//            std::cout << "Dump Z " << FLAGS_z << std::endl;
            //lda->storeZ(FLAGS_z);
            lda->storeDocTopicDistribution(FLAGS_dtdist, FLAGS_k);
        }
    }
    else if(FLAGS_inference)
    {
        lda->loadModel(FLAGS_model);
        lda->inference(FLAGS_niter, FLAGS_perplexity);
        if (FLAGS_dumpz)
        {
            SetIfEmpty(FLAGS_z, FLAGS_prefix + ".z.inference");
 //           std::cout << "Dump Z " << FLAGS_z << std::endl;
            lda->storeZ(FLAGS_z);
        }
    }
	return 0;
}
