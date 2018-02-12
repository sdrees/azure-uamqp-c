# Podspec files (like this one) are Ruby code

Pod::Spec.new do |s|
  s.name             = 'AzureIoTuAmqp'
  s.version          = '1.0.0-pre-release-1.0.0'
  s.summary          = 'AzureIoTuAmqp preview library for CocoaPods.'

  s.description      = <<-DESC
This is a preview CocoaPods 
release of the Azure C uAMQP library.
                       DESC

  s.homepage         = 'https://github.com/azure/azure-uamqp-c'
  s.license          = { :type => 'MIT', :file => 'LICENSE' }
  s.author           = { 'Microsoft' => '' }
  s.source           = { :git => 'https://github.com/Azure/azure-uamqp-c.git', :branch => 'pod' }

  s.ios.deployment_target = '8.0'
  
  s.source_files = 
    'inc/**/*.h',
    'src/*.c' 
    
  s.exclude_files = 
    'src/socket_listener_win32.c'
  
  # The header_mappings_dir is a location where the header files directory structure
  # is preserved.  If not provided the headers files are flattened.
  s.header_mappings_dir = 'inc/'

  s.public_header_files = 'inc/**/*.h'
  
  s.xcconfig = {
    'USE_HEADERMAP' => 'NO',
    'HEADER_SEARCH_PATHS' => '"${PODS_ROOT}/AzureIoTUtility/inc/" "${PODS_ROOT}/AzureIoTuAmqp/inc/"',
    'ALWAYS_SEARCH_USER_PATHS' => 'NO'
  }
  
  s.dependency 'AzureIoTUtility', '1.0.0-pre-release-1.0.0'
end
