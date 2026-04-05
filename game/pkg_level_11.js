
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
      var PACKAGE_NAME = '/Users/johnstiles/Candy Crisis/build-web/pkg_level_11.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_11.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5011", true, true);

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
          Module['removeRunDependency']('datafile_/Users/johnstiles/Candy Crisis/build-web/pkg_level_11.data');
      }
      Module['addRunDependency']('datafile_/Users/johnstiles/Candy Crisis/build-web/pkg_level_11.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5011/Backdrop.png", "start": 0, "end": 282792}, {"filename": "/CandyCrisisResources/PICT_5011/BackdropSolitaire.png", "start": 282792, "end": 597468}, {"filename": "/CandyCrisisResources/PICT_5011/Blob1.png", "start": 597468, "end": 602796}, {"filename": "/CandyCrisisResources/PICT_5011/Blob2.png", "start": 602796, "end": 608118}, {"filename": "/CandyCrisisResources/PICT_5011/Blob3.png", "start": 608118, "end": 614298}, {"filename": "/CandyCrisisResources/PICT_5011/Board.png", "start": 614298, "end": 618181}, {"filename": "/CandyCrisisResources/PICT_5011/BoardGrays.png", "start": 618181, "end": 619332}, {"filename": "/CandyCrisisResources/PICT_5011/BoardNext.png", "start": 619332, "end": 623282}, {"filename": "/CandyCrisisResources/PICT_5011/BoardScore.png", "start": 623282, "end": 624356}, {"filename": "/CandyCrisisResources/PICT_5011/BoardSelect.png", "start": 624356, "end": 684370}, {"filename": "/CandyCrisisResources/PICT_5011/Fireball1.png", "start": 684370, "end": 687955}, {"filename": "/CandyCrisisResources/PICT_5011/Fireball2.png", "start": 687955, "end": 691500}, {"filename": "/CandyCrisisResources/PICT_5011/Fireball3.png", "start": 691500, "end": 694854}, {"filename": "/CandyCrisisResources/PICT_5011/Fireball4.png", "start": 694854, "end": 698361}, {"filename": "/CandyCrisisResources/PICT_5011/H-1A.png", "start": 698361, "end": 813681}, {"filename": "/CandyCrisisResources/PICT_5011/H-2A.png", "start": 813681, "end": 929013}, {"filename": "/CandyCrisisResources/PICT_5011/H-3A.png", "start": 929013, "end": 1044338}, {"filename": "/CandyCrisisResources/PICT_5011/H-4A.png", "start": 1044338, "end": 1159552}, {"filename": "/CandyCrisisResources/PICT_5011/H-5A.png", "start": 1159552, "end": 1274271}, {"filename": "/CandyCrisisResources/PICT_5011/H-6A.png", "start": 1274271, "end": 1389591}, {"filename": "/CandyCrisisResources/PICT_5011/N-1A.png", "start": 1389591, "end": 1503832}, {"filename": "/CandyCrisisResources/PICT_5011/N-2A.png", "start": 1503832, "end": 1618664}, {"filename": "/CandyCrisisResources/PICT_5011/N-3A.png", "start": 1618664, "end": 1733168}, {"filename": "/CandyCrisisResources/PICT_5011/N-4A.png", "start": 1733168, "end": 1848285}, {"filename": "/CandyCrisisResources/PICT_5011/N-5A.png", "start": 1848285, "end": 1962379}, {"filename": "/CandyCrisisResources/PICT_5011/Poof1.png", "start": 1962379, "end": 1963400}, {"filename": "/CandyCrisisResources/PICT_5011/Poof2.png", "start": 1963400, "end": 1964548}, {"filename": "/CandyCrisisResources/PICT_5011/Poof3.png", "start": 1964548, "end": 1966253}, {"filename": "/CandyCrisisResources/PICT_5011/S-1A.png", "start": 1966253, "end": 2083772}, {"filename": "/CandyCrisisResources/PICT_5011/S-2A.png", "start": 2083772, "end": 2201370}, {"filename": "/CandyCrisisResources/PICT_5011/S-3A.png", "start": 2201370, "end": 2319001}, {"filename": "/CandyCrisisResources/PICT_5011/S-4A.png", "start": 2319001, "end": 2436491}, {"filename": "/CandyCrisisResources/PICT_5011/S-5A.png", "start": 2436491, "end": 2553516}, {"filename": "/CandyCrisisResources/PICT_5011/S-6A.png", "start": 2553516, "end": 2671035}, {"filename": "/CandyCrisisResources/PICT_5011/Score.png", "start": 2671035, "end": 2678867}, {"filename": "/CandyCrisisResources/PICT_5011/ScoreInnerMask.png", "start": 2678867, "end": 2679965}, {"filename": "/CandyCrisisResources/PICT_5011/ScoreOuterMask.png", "start": 2679965, "end": 2681094}, {"filename": "/CandyCrisisResources/mod_139", "start": 2681094, "end": 2897212}], "remote_package_size": 2897212});

  })();
