
  var Module = typeof Module != 'undefined' ? Module : {};

  if (!Module['expectedDataFileDownloads']) Module['expectedDataFileDownloads'] = 0;
  Module['expectedDataFileDownloads']++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    var isNode = globalThis.process && globalThis.process.versions && globalThis.process.versions.node && globalThis.process.type != 'renderer';
    async function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'pkg_shared.data';
      var REMOTE_PACKAGE_BASE = 'pkg_shared.data';
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
      var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      async function fetchRemotePackage(packageName, packageSize) {
        if (isNode) {
          var contents = require('fs').readFileSync(packageName);
          return new Uint8Array(contents).buffer;
        }
        if (!Module['dataFileDownloads']) Module['dataFileDownloads'] = {};
        try {
          var response = await fetch(packageName);
        } catch (e) {
          throw new Error(`Network Error: ${packageName}`, {e});
        }
        if (!response.ok) {
          throw new Error(`${response.status}: ${response.url}`);
        }

        const chunks = [];
        const headers = response.headers;
        const total = Number(headers.get('Content-Length') || packageSize);
        let loaded = 0;

        Module['setStatus'] && Module['setStatus']('Downloading data...');
        const reader = response.body.getReader();

        while (1) {
          var {done, value} = await reader.read();
          if (done) break;
          chunks.push(value);
          loaded += value.length;
          Module['dataFileDownloads'][packageName] = {loaded, total};

          let totalLoaded = 0;
          let totalSize = 0;

          for (const download of Object.values(Module['dataFileDownloads'])) {
            totalLoaded += download.loaded;
            totalSize += download.total;
          }

          Module['setStatus'] && Module['setStatus'](`Downloading data... (${totalLoaded}/${totalSize})`);
        }

        const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
        let offset = 0;
        for (const chunk of chunks) {
          packageData.set(chunk, offset);
          offset += chunk.length;
        }
        return packageData.buffer;
      }

      var fetchPromise;
      var fetched = Module['getPreloadedPackage'] && Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);

      if (!fetched) {
        // Note that we don't use await here because we want to execute the
        // the rest of this function immediately.
        fetchPromise = fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);
      }

    async function runWithFS(Module) {

      function assert(check, msg) {
        if (!check) throw new Error(msg);
      }
Module['FS_createPath']("/", "CandyCrisisResources", true, true);

    for (var file of metadata['files']) {
      var name = file['filename']
      Module['addRunDependency'](`fp ${name}`);
    }

      async function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData ' + arrayBuffer.constructor.name);
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          for (var file of metadata['files']) {
            var name = file['filename'];
            var data = byteArray.subarray(file['start'], file['end']);
            // canOwn this data in the filesystem, it is a slice into the heap that will never change
        Module['FS_createDataFile'](name, null, data, true, true, true);
        Module['removeRunDependency'](`fp ${name}`);
          }
          Module['removeRunDependency']('datafile_pkg_shared.data');
      }
      Module['addRunDependency']('datafile_pkg_shared.data');

      if (!Module['preloadResults']) Module['preloadResults'] = {};

      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (!fetched) {
        fetched = await fetchPromise;
      }
      processPackageData(fetched);

    }
    if (Module['calledRun']) {
      runWithFS(Module);
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module['preRun'].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/CandyCrisisResources/CandyCrisisLevels.txt", "start": 0, "end": 816}, {"filename": "/CandyCrisisResources/PICT_200.png", "start": 816, "end": 627770}, {"filename": "/CandyCrisisResources/PICT_201.png", "start": 627770, "end": 671837}, {"filename": "/CandyCrisisResources/PICT_202.png", "start": 671837, "end": 686723}, {"filename": "/CandyCrisisResources/PICT_206.png", "start": 686723, "end": 998427}, {"filename": "/CandyCrisisResources/PICT_207.png", "start": 998427, "end": 1126044}, {"filename": "/CandyCrisisResources/PICT_250.png", "start": 1126044, "end": 1136701}, {"filename": "/CandyCrisisResources/PICT_251.png", "start": 1136701, "end": 1154410}, {"filename": "/CandyCrisisResources/PICT_252.png", "start": 1154410, "end": 1166847}, {"filename": "/CandyCrisisResources/PICT_254.png", "start": 1166847, "end": 1170749}, {"filename": "/CandyCrisisResources/PICT_255.png", "start": 1170749, "end": 1177415}, {"filename": "/CandyCrisisResources/PICT_256.png", "start": 1177415, "end": 1179529}, {"filename": "/CandyCrisisResources/PICT_257.png", "start": 1179529, "end": 1179619}, {"filename": "/CandyCrisisResources/PICT_258.png", "start": 1179619, "end": 1221807}, {"filename": "/CandyCrisisResources/PICT_300.png", "start": 1221807, "end": 1384915}, {"filename": "/CandyCrisisResources/PICT_302.png", "start": 1384915, "end": 1496582}, {"filename": "/CandyCrisisResources/PICT_303.jpg", "start": 1496582, "end": 2036958}, {"filename": "/CandyCrisisResources/PICT_400.png", "start": 2036958, "end": 2037385}, {"filename": "/CandyCrisisResources/PICT_401.png", "start": 2037385, "end": 2145993}, {"filename": "/CandyCrisisResources/PICT_402.png", "start": 2145993, "end": 2171492}, {"filename": "/CandyCrisisResources/PICT_403.png", "start": 2171492, "end": 2612885}, {"filename": "/CandyCrisisResources/PICT_404.png", "start": 2612885, "end": 2667493}, {"filename": "/CandyCrisisResources/PICT_500.png", "start": 2667493, "end": 2690263}, {"filename": "/CandyCrisisResources/PICT_501.png", "start": 2690263, "end": 2732966}, {"filename": "/CandyCrisisResources/PICT_502.png", "start": 2732966, "end": 2767463}, {"filename": "/CandyCrisisResources/PICT_503.png", "start": 2767463, "end": 2811116}, {"filename": "/CandyCrisisResources/PICT_504.png", "start": 2811116, "end": 2853857}, {"filename": "/CandyCrisisResources/PICT_510.png", "start": 2853857, "end": 2854758}, {"filename": "/CandyCrisisResources/PICT_511.png", "start": 2854758, "end": 2858415}, {"filename": "/CandyCrisisResources/PICT_512.png", "start": 2858415, "end": 2866228}, {"filename": "/CandyCrisisResources/PICT_513.png", "start": 2866228, "end": 2877389}, {"filename": "/CandyCrisisResources/PICT_514.png", "start": 2877389, "end": 2892922}, {"filename": "/CandyCrisisResources/PICT_520.png", "start": 2892922, "end": 2956470}, {"filename": "/CandyCrisisResources/PICT_530.png", "start": 2956470, "end": 2959765}, {"filename": "/CandyCrisisResources/PICT_531.png", "start": 2959765, "end": 2963130}, {"filename": "/CandyCrisisResources/PICT_532.png", "start": 2963130, "end": 2967030}, {"filename": "/CandyCrisisResources/PICT_533.png", "start": 2967030, "end": 2971118}, {"filename": "/CandyCrisisResources/PICT_534.png", "start": 2971118, "end": 2974088}, {"filename": "/CandyCrisisResources/PICT_535.png", "start": 2974088, "end": 2977159}, {"filename": "/CandyCrisisResources/PICT_536.png", "start": 2977159, "end": 2978822}, {"filename": "/CandyCrisisResources/PICT_537.png", "start": 2978822, "end": 2981668}, {"filename": "/CandyCrisisResources/PICT_538.png", "start": 2981668, "end": 2984486}, {"filename": "/CandyCrisisResources/PICT_539.png", "start": 2984486, "end": 2988100}, {"filename": "/CandyCrisisResources/PICT_540.png", "start": 2988100, "end": 2990516}, {"filename": "/CandyCrisisResources/PICT_541.png", "start": 2990516, "end": 2993600}, {"filename": "/CandyCrisisResources/PICT_542.png", "start": 2993600, "end": 2995760}, {"filename": "/CandyCrisisResources/PICT_543.png", "start": 2995760, "end": 2997203}, {"filename": "/CandyCrisisResources/PICT_544.png", "start": 2997203, "end": 3007550}, {"filename": "/CandyCrisisResources/mod_128", "start": 3007550, "end": 3112370}, {"filename": "/CandyCrisisResources/mod_140", "start": 3112370, "end": 3273742}, {"filename": "/CandyCrisisResources/mod_141", "start": 3273742, "end": 3515866}, {"filename": "/CandyCrisisResources/mod_142", "start": 3515866, "end": 3904856}, {"filename": "/CandyCrisisResources/snd_128.wav", "start": 3904856, "end": 3905260}, {"filename": "/CandyCrisisResources/snd_129.wav", "start": 3905260, "end": 3907762}, {"filename": "/CandyCrisisResources/snd_130.wav", "start": 3907762, "end": 3910936}, {"filename": "/CandyCrisisResources/snd_131.wav", "start": 3910936, "end": 3918990}, {"filename": "/CandyCrisisResources/snd_132.wav", "start": 3918990, "end": 3927454}, {"filename": "/CandyCrisisResources/snd_133.wav", "start": 3927454, "end": 3928498}, {"filename": "/CandyCrisisResources/snd_134.wav", "start": 3928498, "end": 3946656}, {"filename": "/CandyCrisisResources/snd_135.wav", "start": 3946656, "end": 3949896}, {"filename": "/CandyCrisisResources/snd_136.wav", "start": 3949896, "end": 4011656}, {"filename": "/CandyCrisisResources/snd_137.wav", "start": 4011656, "end": 4041536}, {"filename": "/CandyCrisisResources/snd_138.wav", "start": 4041536, "end": 4047826}, {"filename": "/CandyCrisisResources/snd_139.wav", "start": 4047826, "end": 4050800}, {"filename": "/CandyCrisisResources/snd_140.wav", "start": 4050800, "end": 4069470}, {"filename": "/CandyCrisisResources/snd_141.wav", "start": 4069470, "end": 4071734}, {"filename": "/CandyCrisisResources/snd_142.wav", "start": 4071734, "end": 4086724}, {"filename": "/CandyCrisisResources/snd_143.wav", "start": 4086724, "end": 4129652}, {"filename": "/CandyCrisisResources/snd_144.wav", "start": 4129652, "end": 4138656}], "remote_package_size": 4138656});

  })();
