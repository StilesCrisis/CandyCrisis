
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
      var PACKAGE_NAME = 'pkg_level_4.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_4.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5004", true, true);

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
          Module['removeRunDependency']('datafile_pkg_level_4.data');
      }
      Module['addRunDependency']('datafile_pkg_level_4.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5004/Backdrop.png", "start": 0, "end": 347297}, {"filename": "/CandyCrisisResources/PICT_5004/BackdropPuppet.png", "start": 347297, "end": 545477}, {"filename": "/CandyCrisisResources/PICT_5004/Board.png", "start": 545477, "end": 551077}, {"filename": "/CandyCrisisResources/PICT_5004/BoardGrays.png", "start": 551077, "end": 552932}, {"filename": "/CandyCrisisResources/PICT_5004/BoardNext.png", "start": 552932, "end": 556899}, {"filename": "/CandyCrisisResources/PICT_5004/BoardScore.png", "start": 556899, "end": 558616}, {"filename": "/CandyCrisisResources/PICT_5004/BoardSelect.png", "start": 558616, "end": 619128}, {"filename": "/CandyCrisisResources/PICT_5004/H-1A.png", "start": 619128, "end": 625996}, {"filename": "/CandyCrisisResources/PICT_5004/H-2A.png", "start": 625996, "end": 632952}, {"filename": "/CandyCrisisResources/PICT_5004/H-3A.png", "start": 632952, "end": 642118}, {"filename": "/CandyCrisisResources/PICT_5004/H-4A.png", "start": 642118, "end": 650753}, {"filename": "/CandyCrisisResources/PICT_5004/H-5A.png", "start": 650753, "end": 659190}, {"filename": "/CandyCrisisResources/PICT_5004/H-6A.png", "start": 659190, "end": 666058}, {"filename": "/CandyCrisisResources/PICT_5004/N-1A.png", "start": 666058, "end": 668458}, {"filename": "/CandyCrisisResources/PICT_5004/N-2A.png", "start": 668458, "end": 670882}, {"filename": "/CandyCrisisResources/PICT_5004/N-3A.png", "start": 670882, "end": 674148}, {"filename": "/CandyCrisisResources/PICT_5004/N-4A.png", "start": 674148, "end": 681500}, {"filename": "/CandyCrisisResources/PICT_5004/N-5A.png", "start": 681500, "end": 689716}, {"filename": "/CandyCrisisResources/PICT_5004/S-1A.png", "start": 689716, "end": 692296}, {"filename": "/CandyCrisisResources/PICT_5004/S-2A.png", "start": 692296, "end": 694968}, {"filename": "/CandyCrisisResources/PICT_5004/S-3A.png", "start": 694968, "end": 697686}, {"filename": "/CandyCrisisResources/PICT_5004/S-4A.png", "start": 697686, "end": 701721}, {"filename": "/CandyCrisisResources/PICT_5004/S-5A.png", "start": 701721, "end": 708500}, {"filename": "/CandyCrisisResources/PICT_5004/S-6A.png", "start": 708500, "end": 711080}, {"filename": "/CandyCrisisResources/PICT_5004/Score.png", "start": 711080, "end": 718989}, {"filename": "/CandyCrisisResources/PICT_5004/ScoreInnerMask.png", "start": 718989, "end": 720087}, {"filename": "/CandyCrisisResources/PICT_5004/ScoreOuterMask.png", "start": 720087, "end": 721216}, {"filename": "/CandyCrisisResources/PICT_5004/SquidPuppetsLittle.png", "start": 721216, "end": 764084}, {"filename": "/CandyCrisisResources/PICT_5004/X-UA.png", "start": 764084, "end": 819811}, {"filename": "/CandyCrisisResources/PICT_5004/X-UB.png", "start": 819811, "end": 876145}, {"filename": "/CandyCrisisResources/PICT_5004/X-UC.png", "start": 876145, "end": 933242}, {"filename": "/CandyCrisisResources/PICT_5004/X-UD.png", "start": 933242, "end": 989576}, {"filename": "/CandyCrisisResources/mod_132", "start": 989576, "end": 1396432}], "remote_package_size": 1396432});

  })();
