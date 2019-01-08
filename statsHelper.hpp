double get_base(unsigned N, double skew) {
  double base = 0;
  for (unsigned k = 1; k <= N; k++) {
    base += pow(k, -1 * skew);
  }
  return (1 / base);
}

double get_zipf_prob(unsigned rank, double skew, double base) {
  return pow(rank, -1 * skew) / base;
}

int sample(int n, unsigned& seed, double base,
           std::unordered_map<unsigned, double>& sum_probs) {
  double z;            // Uniform random number (0 < z < 1)
  int zipf_value;      // Computed exponential value to be returned
  int i;               // Loop counter
  int low, high, mid;  // Binary-search bounds

  // Pull a uniform random number (0 < z < 1)
  do {
    z = rand_r(&seed) / static_cast<double>(RAND_MAX);
  } while ((z == 0) || (z == 1));

  // Map z to the value
  low = 1, high = n;

  do {
    mid = floor((low + high) / 2);
    if (sum_probs[mid] >= z && sum_probs[mid - 1] < z) {
      zipf_value = mid;
      break;
    } else if (sum_probs[mid] >= z) {
      high = mid - 1;
    } else {
      low = mid + 1;
    }
  } while (low <= high);

  // Assert that zipf_value is between 1 and N
  assert((zipf_value >= 1) && (zipf_value <= n));
  return zipf_value;
}

std::vector<std::string> generateZipWorkload(unsigned num_keys, double zipf, unsigned seed) {
	std::unordered_map<unsigned, double> sum_probs;
	std::vector<std::string> keys;
	double base;
    base = get_base(num_keys, zipf);
    sum_probs[0] = 0;
    for (unsigned i = 1; i <= num_keys; i++) {
    	sum_probs[i] = sum_probs[i - 1] + base / pow((double)i, zipf);
    }

    std::string key;
    unsigned k;
    unsigned i = 0;
    while (i < num_keys) {
    	k = sample(num_keys, seed, base, sum_probs);
    	key = std::string(8 - std::to_string(k).length(), '0') + std::to_string(k);
    	keys.push_back(key);
    	i++;
    }
    return keys;
}


std::unordered_map<std::string, int> computeFrequencies(std::vector<std::string> requests) {
  std::unordered_map<std::string, int> frequencies;
  for (std::string n: requests) {
    if (frequencies.find(n) == frequencies.end()) {
      frequencies[n] = 1;
    } else {
      frequencies[n] = frequencies[n] + 1;
    }
  }
  return frequencies;
}

std::pair<double, double> computeStats(std::unordered_map<std::string, int> freqMap) {
    unsigned cnt = 0;
    double mean = 0;
    double ms = 0;

    for (const auto& key_access_pair : freqMap) {
        int total_access = key_access_pair.second;

        if (total_access > 0) {
            cnt += 1;

            double delta = total_access - mean;
            mean += (double)delta / cnt;

            double delta2 = total_access - mean;
            ms += delta * delta2;
        }
    }
    double std = sqrt((double)ms / cnt);
    std::pair<double, double> stats = std::make_pair(mean, std::pow(std, 2));
    return stats;
}