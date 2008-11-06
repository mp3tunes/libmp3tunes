Name: libmp3tunes	
Version: 1.0
Release: 1%{?dist}
Summary: A library for interfacing with the MP3tunes Music Locker

Group: Networking/Libraries
License: GPL
URL: http://www.mp3tunes.com		
Source0: libmp3tunes-1.0.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires: loudmouth-devel
Requires: loudmouth



%description


%package devel
Summary: Development files for libmp3tunes
Group: Development/Libraries

%description devel

%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
rm -f $RPM_BUILD_ROOT%{_libdir}/libmp3tunes.la


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc
%{_libdir}/libmp3tunes*.so.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/libmp3tunes*.so
%{_libdir}/libmp3tunes*.a
%{_libdir}/pkgconfig/*
%{_prefix}/include/libmp3tunes



%changelog

