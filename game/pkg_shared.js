
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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/CandyCrisisLevels.txt", "start": 0, "end": 816}, {"filename": "/CandyCrisisResources/PICT_200.png", "start": 816, "end": 627770}, {"filename": "/CandyCrisisResources/PICT_201.png", "start": 627770, "end": 671837}, {"filename": "/CandyCrisisResources/PICT_202.png", "start": 671837, "end": 686723}, {"filename": "/CandyCrisisResources/PICT_206.png", "start": 686723, "end": 998427}, {"filename": "/CandyCrisisResources/PICT_207.png", "start": 998427, "end": 1126044}, {"filename": "/CandyCrisisResources/PICT_250.png", "start": 1126044, "end": 1136701}, {"filename": "/CandyCrisisResources/PICT_251.png", "start": 1136701, "end": 1154410}, {"filename": "/CandyCrisisResources/PICT_252.png", "start": 1154410, "end": 1166847}, {"filename": "/CandyCrisisResources/PICT_254.png", "start": 1166847, "end": 1170749}, {"filename": "/CandyCrisisResources/PICT_255.png", "start": 1170749, "end": 1177415}, {"filename": "/CandyCrisisResources/PICT_256.png", "start": 1177415, "end": 1179529}, {"filename": "/CandyCrisisResources/PICT_257.png", "start": 1179529, "end": 1179619}, {"filename": "/CandyCrisisResources/PICT_258.png", "start": 1179619, "end": 1221807}, {"filename": "/CandyCrisisResources/PICT_300.png", "start": 1221807, "end": 1384915}, {"filename": "/CandyCrisisResources/PICT_302.png", "start": 1384915, "end": 1496582}, {"filename": "/CandyCrisisResources/PICT_303.jpg", "start": 1496582, "end": 2036958}, {"filename": "/CandyCrisisResources/PICT_400.png", "start": 2036958, "end": 2037385}, {"filename": "/CandyCrisisResources/PICT_401.png", "start": 2037385, "end": 2145993}, {"filename": "/CandyCrisisResources/PICT_402.png", "start": 2145993, "end": 2171492}, {"filename": "/CandyCrisisResources/PICT_403.png", "start": 2171492, "end": 2612885}, {"filename": "/CandyCrisisResources/PICT_404.png", "start": 2612885, "end": 2667493}, {"filename": "/CandyCrisisResources/PICT_500.png", "start": 2667493, "end": 2690263}, {"filename": "/CandyCrisisResources/PICT_501.png", "start": 2690263, "end": 2727064}, {"filename": "/CandyCrisisResources/PICT_502.png", "start": 2727064, "end": 2755872}, {"filename": "/CandyCrisisResources/PICT_503.png", "start": 2755872, "end": 2793492}, {"filename": "/CandyCrisisResources/PICT_504.png", "start": 2793492, "end": 2830091}, {"filename": "/CandyCrisisResources/PICT_510.png", "start": 2830091, "end": 2830992}, {"filename": "/CandyCrisisResources/PICT_511.png", "start": 2830992, "end": 2834649}, {"filename": "/CandyCrisisResources/PICT_512.png", "start": 2834649, "end": 2842462}, {"filename": "/CandyCrisisResources/PICT_513.png", "start": 2842462, "end": 2853623}, {"filename": "/CandyCrisisResources/PICT_514.png", "start": 2853623, "end": 2869156}, {"filename": "/CandyCrisisResources/PICT_520.png", "start": 2869156, "end": 2932704}, {"filename": "/CandyCrisisResources/PICT_530.png", "start": 2932704, "end": 2935999}, {"filename": "/CandyCrisisResources/PICT_531.png", "start": 2935999, "end": 2939364}, {"filename": "/CandyCrisisResources/PICT_532.png", "start": 2939364, "end": 2943264}, {"filename": "/CandyCrisisResources/PICT_533.png", "start": 2943264, "end": 2947352}, {"filename": "/CandyCrisisResources/PICT_534.png", "start": 2947352, "end": 2950322}, {"filename": "/CandyCrisisResources/PICT_535.png", "start": 2950322, "end": 2953393}, {"filename": "/CandyCrisisResources/PICT_536.png", "start": 2953393, "end": 2955056}, {"filename": "/CandyCrisisResources/PICT_537.png", "start": 2955056, "end": 2957902}, {"filename": "/CandyCrisisResources/PICT_538.png", "start": 2957902, "end": 2960720}, {"filename": "/CandyCrisisResources/PICT_539.png", "start": 2960720, "end": 2964334}, {"filename": "/CandyCrisisResources/PICT_540.png", "start": 2964334, "end": 2966750}, {"filename": "/CandyCrisisResources/PICT_541.png", "start": 2966750, "end": 2969834}, {"filename": "/CandyCrisisResources/PICT_542.png", "start": 2969834, "end": 2971994}, {"filename": "/CandyCrisisResources/PICT_543.png", "start": 2971994, "end": 2973437}, {"filename": "/CandyCrisisResources/PICT_544.png", "start": 2973437, "end": 2983784}, {"filename": "/CandyCrisisResources/mod_128", "start": 2983784, "end": 3088604}, {"filename": "/CandyCrisisResources/mod_140", "start": 3088604, "end": 3249976}, {"filename": "/CandyCrisisResources/mod_141", "start": 3249976, "end": 3492100}, {"filename": "/CandyCrisisResources/mod_142", "start": 3492100, "end": 3881090}, {"filename": "/CandyCrisisResources/snd_128.wav", "start": 3881090, "end": 3881494}, {"filename": "/CandyCrisisResources/snd_129.wav", "start": 3881494, "end": 3883996}, {"filename": "/CandyCrisisResources/snd_130.wav", "start": 3883996, "end": 3887170}, {"filename": "/CandyCrisisResources/snd_131.wav", "start": 3887170, "end": 3895224}, {"filename": "/CandyCrisisResources/snd_132.wav", "start": 3895224, "end": 3903688}, {"filename": "/CandyCrisisResources/snd_133.wav", "start": 3903688, "end": 3904732}, {"filename": "/CandyCrisisResources/snd_134.wav", "start": 3904732, "end": 3922890}, {"filename": "/CandyCrisisResources/snd_135.wav", "start": 3922890, "end": 3926130}, {"filename": "/CandyCrisisResources/snd_136.wav", "start": 3926130, "end": 3987890}, {"filename": "/CandyCrisisResources/snd_137.wav", "start": 3987890, "end": 4017770}, {"filename": "/CandyCrisisResources/snd_138.wav", "start": 4017770, "end": 4024060}, {"filename": "/CandyCrisisResources/snd_139.wav", "start": 4024060, "end": 4027034}, {"filename": "/CandyCrisisResources/snd_140.wav", "start": 4027034, "end": 4045704}, {"filename": "/CandyCrisisResources/snd_141.wav", "start": 4045704, "end": 4047968}, {"filename": "/CandyCrisisResources/snd_142.wav", "start": 4047968, "end": 4062958}, {"filename": "/CandyCrisisResources/snd_143.wav", "start": 4062958, "end": 4105886}, {"filename": "/CandyCrisisResources/snd_144.wav", "start": 4105886, "end": 4114890}], "remote_package_size": 4114890});

  })();
